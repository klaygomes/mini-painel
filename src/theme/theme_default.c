#include "theme.h"

const xf_theme_t xf_theme_default = {
    .background     = XF_RGB(0xFFFFFF),
    .text_primary   = XF_RGB(0x1a1a1a),
    .text_secondary = XF_RGB(0x444441),
    .text_muted     = XF_RGB(0x6b6b67),
    .text_faint     = XF_RGB(0x9a9a95),
    .text_dimmed    = XF_RGB(0x5F5E5A),

    .surface_card      = XF_RGB(0xf1efe8),
    .surface_separator = XF_RGBA(0x000000, 0.08),
    .surface_border    = XF_RGB(0xD3D1C7),
    .white             = XF_RGB(0xFFFFFF),
    .offline           = XF_RGB(0xB4B2A9),

    .danger          = XF_RGB(0xE24B4A),
    .danger_bg       = XF_RGB(0xFCEBEB),
    .danger_pill_bg  = XF_RGB(0xF7C1C1),
    .danger_pill_fg  = XF_RGB(0x791F1F),
    .danger_title_fg = XF_RGB(0x501313),

    .warning          = XF_RGB(0xEF9F27),
    .warning_bg       = XF_RGB(0xFAEEDA),
    .warning_pill_bg  = XF_RGB(0xFAC775),
    .warning_pill_fg  = XF_RGB(0x633806),
    .warning_title_fg = XF_RGB(0x412402),

    .success    = XF_RGB(0x1D9E75),
    .success_bg = XF_RGB(0xEAF3DE),
    .success_fg = XF_RGB(0x27500A),

    .info      = XF_RGB(0x378ADD),
    .info_bg   = XF_RGB(0xE6F1FB),
    .info_fg   = XF_RGB(0x185FA5),
    .info_dark = XF_RGB(0x042C53),
    .info_fill = XF_RGBA(0x378ADD, 0.14),

    .accent    = XF_RGB(0x7F77DD),
    .accent_bg = XF_RGB(0xEEEDFE),
    .accent_fg = XF_RGB(0x3C3489),

    .orange           = XF_RGB(0xD85A30),
    .purple_bar       = XF_RGB(0x534AB7),
    .deploy_bar       = XF_RGB(0x639922),
    .deploy_chip_bg   = XF_RGBA(0x639922, 0.18),
    .deploy_text      = XF_RGB(0x3B6D11),
    .deploy_text_dark = XF_RGB(0x173404),

    .font_sans = "sans-serif",
    .font_mono = "monospace",
};
