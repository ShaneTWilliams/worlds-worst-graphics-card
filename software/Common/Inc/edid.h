#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define EDID_BASIC_SIZE_B 128

typedef struct {
    const uint8_t _data[EDID_BASIC_SIZE_B];
} edid_t;

typedef enum {
    EDID_STATUS_OK = 0x00, //
    EDID_STATUS_NULL_ARG = 0x01,
    EDID_STATUS_NULL_DATA = 0x02,
    EDID_STATUS_CORRUPT = 0x04,
    EDID_STATUS_REQ_FIELD_BLANK = 0x08,
    EDID_STATUS_OPT_FIELD_BLANK = 0x10,
    EDID_STATUS_BAD_FIELD = 0x20
} edid_status_t;

typedef enum {
    EDID_DATE_TYPE_MFR_YEAR,
    EDID_DATE_TYPE_MFR_YEAR_WEEK,
    EDID_DATE_TYPE_MOD_YEAR,
    EDID_DATE_TYPE_CORRUPT
} edid_date_type_t;

typedef enum {
    EDID_VID_SIG_TYPE_ANALOG = 0x00,
    EDID_VID_SIG_TYPE_DIGITAL = 0x01
} edid_vid_sig_type_t;

typedef enum {
    EDID_SIG_LVL_STD_07_03_10 = 0x00,
    EDID_SIG_LVL_STD_0714_0286_10 = 0x01,
    EDID_SIG_LVL_STD_10_04_14 = 0x02,
    EDID_SIG_LVL_STD_07_00_07 = 0x03
} edid_sig_lvl_std_t;

typedef enum {
    EDID_BLANKING_LVL_BLACK = 0x00,
    EDID_BLANKING_LVL_PEDESTAL = 0x01
} edid_blanking_lvl_t;

typedef enum {
    EDID_HVSYNC_SEPERATE_NOT_SUPPORTED = 0x00,
    EDID_HVSYNC_SEPERATE_SUPPORTED = 0x01
} edid_hvsync_seperate_t;

typedef enum {
    EDID_COMP_HSYNC_NOT_SUPPORTED = 0x00,
    EDID_COMP_HSYNC_SUPPORTED = 0x01
} edid_comp_hsync_t;

typedef enum {
    EDID_COMP_GRN_SYNC_NOT_SUPPORTED = 0x00,
    EDID_COMP_GRN_SYNC_SUPPORTED = 0x01
} edid_comp_grn_sync_t;

typedef enum {
    EDID_VSYNC_SERR_NOT_SUPPORTED = 0x00,
    EDID_VSYNC_SERR_SUPPORTED = 0x01
} edid_vsync_serr_t;

typedef enum {
    EDID_COLOR_DEPTH_UNDEFINED = 0x00,
    EDID_COLOR_DEPTH_6_BPC = 0x01,
    EDID_COLOR_DEPTH_8_BPC = 0x02,
    EDID_COLOR_DEPTH_10_BPC = 0x03,
    EDID_COLOR_DEPTH_12_BPC = 0x04,
    EDID_COLOR_DEPTH_14_BPC = 0x05,
    EDID_COLOR_DEPTH_16_BPC = 0x06
} edid_color_depth_t;

typedef enum {
    EDID_VIDEO_IFACE_UNDEFINED = 0x00,
    EDID_VIDEO_IFACE_DVI = 0x01,
    EDID_VIDEO_IFACE_HDMIA = 0x01,
    EDID_VIDEO_IFACE_HDMIB = 0x01,
    EDID_VIDEO_IFACE_MDDI = 0x01,
    EDID_VIDEO_IFACE_DISPLAYPORT = 0x01
} edid_video_iface_t;

typedef enum {
    EDID_RATIO_TYPE_PORTRAIT,
    EDID_RATIO_TYPE_LANDSCAPE,
    EDID_RATIO_TYPE_SIZE,
    EDID_RATIO_TYPE_UNDEFINED
} edid_ratio_type_t;

typedef enum {
    EDID_COLOR_TYPE_MONO_GRAY = 0x00,
    EDID_COLOR_TYPE_RGB = 0x01,
    EDID_COLOR_TYPE_NON_RGB = 0x02,
    EDID_COLOR_TYPE_UNDEFINED = 0x03
} edid_color_type_t;

typedef enum {
    EDID_COLOR_FORMAT_RGB444 = 0x00,
    EDID_COLOR_FORMAT_RGB444_YCRCB444 = 0x01,
    EDID_COLOR_FORMAT_RGB444_YCRCB422 = 0x02,
    EDID_COLOR_FORMAT_RGB444_YCRCB444_YCRCB422 = 0x03,
} edid_color_format_t;

edid_status_t edid_init(edid_t* edid, uint8_t* data);
edid_status_t edid_verify(edid_t* edid);

edid_status_t edid_get_mfr_code(edid_t* edid, uint8_t* code);
edid_status_t edid_get_prod_id(edid_t* edid, uint16_t* id);
edid_status_t edid_get_ser_num(edid_t* edid, uint32_t* ser_num);

edid_status_t edid_get_date_type(edid_t* edid, edid_date_type_t* type);
edid_status_t edid_get_mfr_year_week(edid_t* edid, uint16_t* year, uint8_t* week);
edid_status_t edid_get_model_year(edid_t* edid, uint16_t* year);

edid_status_t edid_get_edid_version(edid_t* edid, uint8_t* version);
edid_status_t edid_get_edid_revision(edid_t* edid, uint8_t* revision);
edid_status_t edid_get_edid_version_revision(edid_t* edid, uint8_t* version, uint8_t* revision);

edid_status_t edid_get_vid_sig_type(edid_t* edid, edid_vid_sig_type_t* type);
edid_status_t edid_get_analog_input_def(edid_t* edid, edid_sig_lvl_std_t* sig_lvl_std,
                                        edid_blanking_lvl_t* blanking_lvl,
                                        edid_hvsync_seperate_t* hvsync_seperate,
                                        edid_comp_hsync_t* comp_hsync,
                                        edid_comp_grn_sync_t* comp_grn_sync,
                                        edid_vsync_serr_t* vsync_serr);
edid_status_t edid_get_digital_input_def(edid_t* edid, edid_color_depth_t* color_depth,
                                         edid_video_iface_t* video_iface);

edid_status_t edid_get_ratio_type(edid_t* edid, edid_ratio_type_t* ratio_type);
edid_status_t edid_get_portrait_ratio(edid_t* edid, float* ratio);
edid_status_t edid_get_landscape_ratio(edid_t* edid, float* ratio);
edid_status_t edid_get_screen_size(edid_t* edid, uint8_t* horiz_size_cm, uint8_t* vert_size_cm);

edid_status_t edid_get_gamma(edid_t* edid, float* gamma);

edid_status_t edid_get_stdby_support(edid_t* edid, bool* stdby_supported);
edid_status_t edid_get_suspend_support(edid_t* edid, bool* suspend_supported);
edid_status_t edid_get_very_low_pwr_support(edid_t* edid, bool* very_low_pwr_supported);
edid_status_t edid_get_color_type(edid_t* edid, edid_color_type_t* color_type);
edid_status_t edid_get_color_format(edid_t* edid, edid_color_format_t* color_format);
edid_status_t edid_get_srgb_default(edid_t* edid, bool* srgb_is_default);
edid_status_t edid_get_timing_has_pxl_fmt_and_refrate(edid_t* edid,
                                                      bool* timing_has_pxl_fmt_and_refrate);
edid_status_t edid_get_continuous_freq(edid_t* edid, bool* freq_is_continuous);

uint8_t edid_buf[EDID_BASIC_SIZE_B] = {
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x10, 0xac, 0x34, 0x12, 0x66, 0x2e, 0x4b, 0x42,
    0x0e, 0x14, 0x01, 0x03, 0x80, 0x33, 0x1d, 0x78, 0x2a, 0x81, 0xf1, 0xa3, 0x57, 0x53, 0x9f, 0x27,
    0x0a, 0x50, 0x54, 0xbf, 0xef, 0x80, 0x81, 0x00, 0x95, 0x00, 0xb3, 0x00, 0x81, 0x40, 0x71, 0x4f,
    0x81, 0x80, 0xa9, 0x40, 0x95, 0x0f, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
    0x45, 0x00, 0xfe, 0x1f, 0x11, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x38, 0x4b, 0x1e,
    0x51, 0x11, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x53,
    0x4d, 0x58, 0x4c, 0x32, 0x33, 0x37, 0x30, 0x48, 0x44, 0x0a, 0x20, 0x20, 0x00, 0x00, 0x00, 0xff,
    0x00, 0x48, 0x31, 0x41, 0x4b, 0x35, 0x30, 0x30, 0x30, 0x30, 0x30, 0x0a, 0x20, 0x20, 0x00, 0xa1};
