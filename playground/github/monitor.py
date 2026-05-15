#!/usr/bin/env python3
"""Poll GitHub repository health using gh CLI and a local JSON config."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Callable

SUCCESS = "SUCCESS"
FAILED = "FAILED"
ERROR = "ERROR"
NONE = "NONE"
NOT_AVAILABLE = "N/A"
GH_TIMEOUT_SECONDS = 20.0
CACHE_TTL_SECONDS = 120.0

_cache: dict[tuple[str, str], tuple[str, float]] = {}

FAIL_CONCLUSIONS = {
    "failure",
    "cancelled",
    "timed_out",
    "action_required",
    "startup_failure",
}

REPO_KEY_PATTERN = re.compile(r"^[^/\s]+/[^/\s]+$")

MetricFn = Callable[..., str]


def _read_json_file(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def load_projects_config(config_path: Path) -> dict[str, dict[str, Any]]:
    """Load and validate projects.json, skipping malformed entries."""
    raw = _read_json_file(config_path)
    if not isinstance(raw, dict):
        raise ValueError(
            "projects.json root must be an object keyed by owner/repo"
        )

    validated: dict[str, dict[str, Any]] = {}
    for repo, settings in raw.items():
        if not isinstance(repo, str) or not REPO_KEY_PATTERN.match(repo):
            print(
                f"Skipping invalid repository key: {repo!r}",
                file=sys.stderr,
            )
            continue
        if not isinstance(settings, dict):
            print(
                f"Skipping {repo}: config value must be an object",
                file=sys.stderr,
            )
            continue

        branch = settings.get("branch")
        ignored_jobs = settings.get("ignored_jobs")

        if not isinstance(branch, str) or not branch.strip():
            print(
                f"Skipping {repo}: 'branch' must be a non-empty string",
                file=sys.stderr,
            )
            continue
        if not isinstance(ignored_jobs, list) or not all(
            isinstance(j, str) for j in ignored_jobs
        ):
            print(
                f"Skipping {repo}: 'ignored_jobs' must be an array of strings",
                file=sys.stderr,
            )
            continue

        validated[repo] = {
            "branch": branch,
            "ignored_jobs": ignored_jobs,
        }

    return validated


def run_gh_json(command: list[str]) -> Any:
    """Run a gh command that includes --json and return parsed JSON output."""
    if "--json" not in command:
        raise ValueError("All gh commands must include --json")

    proc = subprocess.run(
        command,
        check=True,
        capture_output=True,
        text=True,
        timeout=GH_TIMEOUT_SECONDS,
    )

    stdout = proc.stdout.strip()
    if not stdout:
        return None
    try:
        return json.loads(stdout)
    except json.JSONDecodeError as exc:
        raise ValueError(f"Invalid JSON from gh command: {command}") from exc


def _author_from_pr_list(prs: Any, empty_value: str) -> str:
    if not isinstance(prs, list) or not prs:
        return empty_value

    author = prs[0].get("author") if isinstance(prs[0], dict) else None
    if not isinstance(author, dict):
        return empty_value

    login = author.get("login")
    if isinstance(login, str) and login.strip():
        return login

    name = author.get("name")
    if isinstance(name, str) and name.strip():
        return name

    return empty_value


def get_latest_open_pr_author(repo: str) -> str:
    prs = run_gh_json([
        "gh",
        "pr",
        "list",
        "-R",
        repo,
        "--state",
        "open",
        "--limit",
        "1",
        "--json",
        "author",
    ])
    return _author_from_pr_list(prs, NONE)


def get_latest_merged_pr_author(repo: str, branch: str) -> str:
    prs = run_gh_json([
        "gh",
        "pr",
        "list",
        "-R",
        repo,
        "--state",
        "merged",
        "--base",
        branch,
        "--limit",
        "1",
        "--json",
        "author",
    ])
    return _author_from_pr_list(prs, NOT_AVAILABLE)


def get_build_status(repo: str, ignored_jobs: list[str]) -> str:
    runs = run_gh_json([
        "gh",
        "run",
        "list",
        "-R",
        repo,
        "--limit",
        "1",
        "--json",
        "databaseId",
    ])

    if not isinstance(runs, list) or not runs:
        return NOT_AVAILABLE

    run_id = runs[0].get("databaseId") if isinstance(runs[0], dict) else None
    if run_id is None:
        return NOT_AVAILABLE

    run_view = run_gh_json([
        "gh",
        "run",
        "view",
        str(run_id),
        "-R",
        repo,
        "--json",
        "jobs",
    ])

    jobs = run_view.get("jobs") if isinstance(run_view, dict) else None
    if not isinstance(jobs, list):
        return NOT_AVAILABLE

    ignored_set = set(ignored_jobs)
    for job in jobs:
        if not isinstance(job, dict):
            continue
        conclusion = job.get("conclusion")
        name = job.get("name")
        if (
            conclusion in FAIL_CONCLUSIONS
            and (
                not isinstance(name, str)
                or name not in ignored_set
            )
        ):
            return FAILED

    return SUCCESS


def safe_metric(metric_name: str, fn: MetricFn, *args: object) -> str:
    try:
        value = fn(*args)
        if isinstance(value, str):
            return value
        return ERROR
    except subprocess.CalledProcessError as exc:
        stderr = (exc.stderr or "").strip()
        msg = stderr or str(exc)
        print(
            f"{metric_name} ERROR: {msg}",
            file=sys.stderr,
        )
        return ERROR
    except (subprocess.TimeoutExpired, ValueError) as exc:
        msg = str(exc)
        print(
            f"{metric_name} ERROR: {msg}",
            file=sys.stderr,
        )
        return ERROR


def _repo_slug(repo: str) -> str:
    slug = repo.replace("/", "_")
    slug = re.sub(r"[^A-Za-z0-9_-]", "_", slug)
    slug = slug.strip("_")
    if not slug:
        return "repo"
    return slug


def _initials(author: str) -> str:
    if not author or author in {NONE, NOT_AVAILABLE, ERROR}:
        return "NA"
    parts = [part for part in re.split(r"[\s._-]+", author) if part]
    if len(parts) >= 2:
        return (parts[0][0] + parts[1][0]).upper()
    return author[:2].upper()


def _deploy_status_code(status: str) -> int:
    if status == SUCCESS:
        return 1
    if status in {FAILED, ERROR}:
        return 2
    return 0


def _build_ops_from_metrics(
    repo: str,
    branch: str,
    open_author: str,
    merged_author: str,
    build_status: str,
    created_ids: set[str],
) -> list[dict[str, Any]]:
    slug = _repo_slug(repo)
    pr_id = f"component.pr_review.{slug}"
    deploy_id = f"component.deploy.{slug}"

    rows: list[dict[str, Any]] = []
    if open_author not in {NONE, ERROR}:
        rows.append(
            {
                "initials": _initials(open_author),
                "title": f"Open PR by {open_author}",
                "age": "now",
                "reviews": 0,
                "avatar_color": "#4A6EA9",
            }
        )

    if merged_author not in {NOT_AVAILABLE, ERROR}:
        rows.append(
            {
                "initials": _initials(merged_author),
                "title": f"Last merged by {merged_author}",
                "age": "recent",
                "reviews": 0,
                "avatar_color": "#6B8F71",
            }
        )

    if not rows:
        rows.append(
            {
                "initials": "NA",
                "title": "No PR activity",
                "age": NOT_AVAILABLE,
                "reviews": 0,
                "avatar_color": "#7C7C7C",
            }
        )

    pr_payload = {
        "title": f"{repo} PR review",
        "rows": rows[:3],
    }
    deploy_payload = {
        "branch": f"{branch}@latest",
        "time_ago": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "label": build_status.lower(),
        "status": _deploy_status_code(build_status),
    }

    pr_op = "row.add" if pr_id not in created_ids else "row.update"
    deploy_op = "row.add" if deploy_id not in created_ids else "row.update"
    created_ids.add(pr_id)
    created_ids.add(deploy_id)

    return [
        {
            "op": pr_op,
            "id": pr_id,
            "data": pr_payload,
        },
        {
            "op": deploy_op,
            "id": deploy_id,
            "data": deploy_payload,
        },
    ]


def _cached_metric(repo: str, key: str, fn: Callable, *args: object) -> str:
    cache_key = (repo, key)
    entry = _cache.get(cache_key)
    if entry is not None and time.monotonic() - entry[1] < CACHE_TTL_SECONDS:
        return entry[0]
    value = safe_metric(key, fn, *args)
    _cache[cache_key] = (value, time.monotonic())
    return value


def build_json_api_ops(
    repo: str,
    branch: str,
    ignored_jobs: list[str],
    created_ids: set[str],
) -> list[dict[str, Any]]:
    open_author = _cached_metric(
        repo, "open_pr_author", get_latest_open_pr_author, repo,
    )
    merged_author = _cached_metric(
        repo, "merged_pr_author", get_latest_merged_pr_author, repo, branch,
    )
    build_status = _cached_metric(
        repo, "build_status", get_build_status, repo, ignored_jobs,
    )

    return _build_ops_from_metrics(
        repo,
        branch,
        open_author,
        merged_author,
        build_status,
        created_ids,
    )


def _self_check_output_contract() -> int:
    sample_ids: set[str] = set()
    ops = _build_ops_from_metrics(
        repo="owner/repo",
        branch="main",
        open_author="alice",
        merged_author="bob",
        build_status=SUCCESS,
        created_ids=sample_ids,
    )

    if not isinstance(ops, list) or len(ops) != 2:
        print("Self-check failed: expected exactly 2 ops", file=sys.stderr)
        return 1

    required_op_keys = {"op", "id", "data"}
    for op in ops:
        if not isinstance(op, dict) or not required_op_keys.issubset(op):
            print(
                "Self-check failed: op object missing required keys",
                file=sys.stderr,
            )
            return 1

    pr_op, deploy_op = ops
    if not str(pr_op["id"]).startswith("component.pr_review."):
        print(
            "Self-check failed: pr_review id shape mismatch",
            file=sys.stderr,
        )
        return 1
    if not str(deploy_op["id"]).startswith("component.deploy."):
        print(
            "Self-check failed: deploy id shape mismatch",
            file=sys.stderr,
        )
        return 1

    pr_data = pr_op["data"]
    if not isinstance(pr_data, dict):
        print(
            "Self-check failed: pr_review data must be object",
            file=sys.stderr,
        )
        return 1
    if "title" not in pr_data or "rows" not in pr_data:
        print(
            "Self-check failed: pr_review data missing title/rows",
            file=sys.stderr,
        )
        return 1
    if not isinstance(pr_data["rows"], list):
        print(
            "Self-check failed: pr_review rows must be list",
            file=sys.stderr,
        )
        return 1

    deploy_data = deploy_op["data"]
    if not isinstance(deploy_data, dict):
        print(
            "Self-check failed: deploy data must be object",
            file=sys.stderr,
        )
        return 1
    deploy_required = {"branch", "time_ago", "label", "status"}
    if not deploy_required.issubset(deploy_data):
        print(
            "Self-check failed: deploy data missing required keys",
            file=sys.stderr,
        )
        return 1

    print("Self-check passed", file=sys.stderr)
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Monitor repositories using gh CLI"
    )
    parser.add_argument(
        "--config",
        type=Path,
        default=Path(__file__).with_name("projects.json"),
        help="Path to projects.json",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=30.0,
        help="Polling interval in seconds",
    )
    parser.add_argument(
        "--self-check",
        action="store_true",
        help="Run internal output contract checks and exit",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    created_ids: set[str] = set()

    if args.self_check:
        return _self_check_output_contract()

    if args.interval <= 0:
        print("--interval must be > 0", file=sys.stderr)
        return 2

    if not args.config.exists():
        print(f"Config file not found: {args.config}", file=sys.stderr)
        return 2

    try:
        while True:
            try:
                projects = load_projects_config(args.config)
            except (OSError, json.JSONDecodeError, ValueError) as exc:
                print(f"Config ERROR: {exc}", file=sys.stderr)
                time.sleep(args.interval)
                continue

            if not projects:
                print("No valid repositories in config", file=sys.stderr)
                time.sleep(args.interval)
                continue

            for repo, cfg in projects.items():
                ops = build_json_api_ops(
                    repo,
                    cfg["branch"],
                    cfg["ignored_jobs"],
                    created_ids,
                )
                print(
                    json.dumps(
                        ops,
                        separators=(",", ":"),
                        ensure_ascii=True,
                    )
                )

            time.sleep(args.interval)
    except KeyboardInterrupt:
        print("Stopped by user", file=sys.stderr)
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
