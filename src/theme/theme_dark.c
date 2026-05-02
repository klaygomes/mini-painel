#include "theme.h"

const xf_theme_t xf_theme = {
    .background     = XF_RGB(0x121212),
    .text_primary   = XF_RGB(0xE5E5E5),
    .text_secondary = XF_RGB(0xA3A3A3),
    .text_muted     = XF_RGB(0x737373),
    .text_subtle    = XF_RGB(0x525252),

    .surface_card      = XF_RGB(0x1E1E1E),
    .surface_separator = XF_RGBA(0xFFFFFF, 0.12),
    .surface_border    = XF_RGB(0x333333),
    .on_color          = XF_RGB(0x121212),
    .status_offline    = XF_RGB(0x525252),

    .danger           = XF_RGB(0xF87171),
    .danger_bg        = XF_RGB(0x451A1A),
    .danger_badge_bg  = XF_RGB(0x7F1D1D),
    .danger_badge_fg  = XF_RGB(0xFECACA),
    .danger_emphasis  = XF_RGB(0xFEF2F2),

    .warning           = XF_RGB(0xFBBF24),
    .warning_bg        = XF_RGB(0x422006),
    .warning_badge_bg  = XF_RGB(0x78350F),
    .warning_badge_fg  = XF_RGB(0xFDE68A),
    .warning_emphasis  = XF_RGB(0xFFFBEB),

    .success          = XF_RGB(0x34D399),
    .success_bg       = XF_RGB(0x064E3B),
    .success_fg       = XF_RGB(0xA7F3D0),
    .success_badge_bg = XF_RGB(0x065F46),
    .success_badge_fg = XF_RGB(0x6EE7B7),

    .info          = XF_RGB(0x60A5FA),
    .info_bg       = XF_RGB(0x1E3A8A),
    .info_fg       = XF_RGB(0xBFDBFE),
    .info_emphasis = XF_RGB(0xEFF6FF),
    .info_subtle   = XF_RGBA(0x60A5FA, 0.20),

    .accent    = XF_RGB(0xA78BFA),
    .accent_bg = XF_RGB(0x2E1065),
    .accent_fg = XF_RGB(0xDDD6FE),

    .caution      = XF_RGB(0xFB923C),
    .chart_accent = XF_RGB(0x818CF8),

    .deploy_bg          = XF_RGB(0x84CC16),
    .deploy_surface     = XF_RGBA(0x84CC16, 0.25),
    .deploy_fg          = XF_RGB(0xD9F99D),
    .deploy_fg_emphasis = XF_RGB(0xECFCCB),

    .font_sans = "sans-serif",
    .font_mono = "monospace",
};