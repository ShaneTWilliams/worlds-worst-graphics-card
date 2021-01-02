#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
/*******************
 ***** GENERAL *****
 *******************/

// State struct.
typedef struct {
	I2C_HandleTypeDef *_i2c_handle;
	uint8_t _i2c_timeout;
	uint8_t _i2c_addr;
} sii1136_t;

// Status; return type of all API functions.
typedef enum {
	SII1136_STATUS_OK, SII1136_STATUS_I2C_ERR, SII1136_STATUS_NULL_ARG
} sii1136_status_t;

// TPI status.
typedef enum {
	SII1136_TPI_STATUS_READY, SII1136_TPI_STATUS_BAD_ID, SII1136_TPI_STATUS_I2C_ERR
} sii1136_tpi_status_t;

const uint8_t SII1136_TPI_ADDR_LOW = 0x72;  // I2C address if CI2CA is held low.
const uint8_t SII1136_TPI_ADDR_HIGH = 0x76; // I2C address if CI2CA is held high.

/***************************
 ***** REGISTER VALUES *****
 ***************************/

/***** INPUT VIDEO FORMAT *****/

// Ratio of TMDS clock to input video clock.
typedef enum {
	SII1136_TMDS_CLK_RATIO_0_5 = 0x00,
	SII1136_TMDS_CLK_RATIO_1 = 0x01,
	SII1136_TMDS_CLK_RATIO_2 = 0x02,
	SII1136_TMDS_CLK_RATIO_4 = 0x03
} sii1136_tmds_clk_ratio_t;

// Input bus pixel width.
typedef enum {
	SII1136_BUS_PXL_WIDTH_HALF = 0x00, SII1136_BUS_PXL_WIDTH_FULL = 0x01
} sii1136_bus_pxl_width_t;

// Video clock edge select.
typedef enum {
	SII1136_VIDEO_CLK_EDGE_FALLING = 0x00, SII1136_VIDEO_CLK_EDGE_RISING = 0x01
} sii1136_video_clk_edge_t;

// Pixel repetition factor.
typedef enum {
	SII1136_PXL_REPETITION_NONE = 0x00,
	SII1136_PXL_REPETITION_TWO = 0x01,
	SII1136_PXL_REPETITION_FOUR = 0x03
} sii1136_pxl_repetition_t;

/***** INPUT COLOR FORMAT *****/

// Input color depth.
typedef enum {
	SII1136_IN_COLOR_DEPTH_8 = 0x00,
	SII1136_IN_COLOR_DEPTH_10_12_UNDITHERED = 0x02,
	SII1136_IN_COLOR_DEPTH_10_12_DITHERED_8 = 0x03
} sii1136_in_color_depth_t;

// Video range expansion.
typedef enum {
	SII1136_VIDEO_RANGE_EXP_AUTO = 0x00,
	SII1136_VIDEO_RANGE_EXP_ON = 0x01,
	SII1136_VIDEO_RANGE_EXP_OFF = 0x02
} sii1136_video_range_exp_t;

// Input color space.
typedef enum {
	SII1136_IN_COLOR_SPACE_RGB = 0x00,
	SII1136_IN_COLOR_SPACE_YCBCR_444 = 0x01,
	SII1136_IN_COLOR_SPACE_YCBCR_422 = 0x02,
	SII1136_IN_COLOR_SPACE_BLACK = 0x03
} sii1136_in_color_space_t;

/***** OUTPUT COLOR FORMAT *****/

// Output color standard.
typedef enum {
	SII1136_OUT_COLOR_STD_BT601 = 0x00, SII1136_OUT_COLOR_STD_BT709 = 0x01
} sii1136_out_color_std_t;

// Video range compression.
typedef enum {
	SII1136_VIDEO_RNG_COMP_AUTO = 0x00,
	SII1136_VIDEO_RNG_COMP_OFF = 0x01,
	SII1136_VIDEO_RNG_COMP_ON = 0x02
} sii1136_video_rng_comp;

// Output color space.
typedef enum {
	SII1136_OUT_COLOR_SPACE_RGB = 0x00,
	SII1136_OUT_COLOR_SPACE_YCBCR_444 = 0x01,
	SII1136_OUT_COLOR_SPACE_YCBCR_422 = 0x02
} sii1136_out_color_space_t;

/***** SYNC CONTROL *****/

// Sync method.
typedef enum {
	SII1136_SYNC_METHOD_EXTERNAL = 0x00, SII1136_SYNC_METHOD_EMBEDDED = 0x01
} sii1136_sync_method_t;

// VBIT adjust type.
typedef enum {
	SII1136_VBIT_ADJ_TYPE_DEC = 0x00, SII1136_VBIT_ADJ_TYPE_INC = 0x01
} sii1136_vbit_adj_type_t;

/***** SYNC POLARITY *****/

// Sync active level.
typedef enum {
	SII1136_SYNC_ACTIVE_LEVEL_HIGH = 0x00, SII1136_SYNC_ACTIVE_LEVEL_LOW = 0x01
} sii11136_sync_act_lvl_t;

/***** YC INPUT *****/

typedef enum {
	SII1136_DDR_BITS_LOWER_12 = 0x00, SII1136_DDR_BITS_UPPER_12 = 0x01
} sii1136_ddr_bits_t;

typedef enum {
	SII1136_YC_IN_MODE_NORM = 0x00,
	SII1136_YC_IN_MODE_NORM_DSWAP = 0x01,
	SII1136_YC_IN_MODE_DDR = 0x02,
	SII1136_YC_IN_MODE_DDR_DSWAP = 0x03,
	SII1136_YC_IN_MODE_24_BIT = 0x04
} sii1136_yc_input_mode_t;

/***** SYSTEM CONTROL *****/

typedef enum {
	SII1136_LINK_INT_MODE_STAT = 0x00, SII1136_LINK_INT_MODE_DYN = 0x01
} sii1136_link_int_mode_t;

typedef enum {
	SII1136_TMDS_OUT_CNTL_ACTIVE = 0x00, SII1136_TMDS_OUT_CNTL_OFF = 0x01
} sii1136_tmds_out_cntl_t;

typedef enum {
	SII1136_OUTPUT_MODE_DVI = 0x00, SII1136_OUTPUT_MODE_HDMI = 0x01
} sii1136_output_mode_t;

/****************************************
 ***** TPI INITIALIZATION FUNCTIONS *****
 ****************************************/

void sii1136_configure_i2c(sii1136_t* self, I2C_HandleTypeDef* i2c, uint8_t i2c_timeout,
							uint8_t i2c_addr);
sii1136_status_t sii1136_init_tpi(sii1136_t* self);
sii1136_status_t sii1136_tpi_ready(sii1136_t* self, sii1136_tpi_status_t* tpi_status);

/********************************************
 ***** REGISTER GETTER/SETTER FUNCTIONS *****
 *******************************************/

/***** IDENTIFICATION REGISTERS *****/

sii1136_status_t sii1136_get_device_id(sii1136_t* self, uint8_t* device_id);
sii1136_status_t sii1136_get_device_rev_id(sii1136_t* self, uint8_t* device_rev_id);
sii1136_status_t sii1136_get_tpi_rev(sii1136_t* self, uint8_t* tpi_revision);

sii1136_status_t sii1136_set_device_id(sii1136_t* self, uint8_t device_id);
sii1136_status_t sii1136_set_device_rev_id(sii1136_t* self, uint8_t device_rev_id);
sii1136_status_t sii1136_set_tpi_rev(sii1136_t* self, uint8_t tpi_revision);

/***** INPUT FORMAT REGISTERS *****/

sii1136_status_t sii1136_get_pixel_clock(sii1136_t* self, uint32_t* pixel_clock);
sii1136_status_t sii1136_get_vert_freq(sii1136_t* self, uint16_t* vert_freq);
sii1136_status_t sii1136_get_horiz_res(sii1136_t* self, uint16_t* horiz_res);
sii1136_status_t sii1136_get_vert_res(sii1136_t* self, uint16_t* vert_res);

sii1136_status_t sii1136_set_pixel_clock(sii1136_t* self, uint32_t pixel_clock);
sii1136_status_t sii1136_set_vert_freq(sii1136_t* self, uint16_t vert_freq);
sii1136_status_t sii1136_set_horiz_res(sii1136_t* self, uint16_t horiz_res);
sii1136_status_t sii1136_set_vert_res(sii1136_t* self, uint16_t vert_res);

/***** PIXEL CLOCK REGISTERS *****/

sii1136_status_t sii1136_get_tmds_clk_ratio(sii1136_t* self,
											sii1136_tmds_clk_ratio_t* tmds_clk_ratio);
sii1136_status_t sii1136_get_bus_pxl_width(sii1136_t* self, sii1136_bus_pxl_width_t* bus_pxl_width);
sii1136_status_t sii1136_get_video_clk_edge(sii1136_t* self,
											sii1136_video_clk_edge_t* video_clk_edge);
sii1136_status_t sii1136_get_pxl_repetition(sii1136_t* self,
											sii1136_pxl_repetition_t* pxl_repetition);
sii1136_status_t sii1136_get_input_format(sii1136_t* self, sii1136_tmds_clk_ratio_t* tmds_clk_ratio,
											sii1136_bus_pxl_width_t* bus_pxl_width,
											sii1136_video_clk_edge_t* video_clk_edge,
											sii1136_pxl_repetition_t* pxl_repetition);

sii1136_status_t sii1136_set_tmds_clk_ratio(sii1136_t* self,
											sii1136_tmds_clk_ratio_t tmds_clk_ratio);
sii1136_status_t sii1136_set_bus_pxl_width(sii1136_t* self, sii1136_bus_pxl_width_t bus_pxl_width);
sii1136_status_t sii1136_set_video_clk_edge(sii1136_t* self,
											sii1136_video_clk_edge_t video_clk_edge);
sii1136_status_t sii1136_set_pxl_repetition(sii1136_t* self,
											sii1136_pxl_repetition_t pxl_repetition);

sii1136_status_t sii1136_set_input_format(sii1136_t* self, sii1136_tmds_clk_ratio_t tmds_clk_ratio,
											sii1136_bus_pxl_width_t bus_pxl_width,
											sii1136_video_clk_edge_t video_clk_edge,
											sii1136_pxl_repetition_t pxl_repetition);

/***** INPUT COLOR FORMAT REGISTERS *****/

sii1136_status_t sii1136_get_in_color_depth(sii1136_t* self,
											sii1136_in_color_depth_t* input_color_depth);
sii1136_status_t sii1136_get_video_range_exp(sii1136_t* self,
												sii1136_video_range_exp_t* video_range_exp);
sii1136_status_t sii1136_get_in_color_space(sii1136_t* self,
											sii1136_in_color_space_t* input_color_space);
sii1136_status_t sii1136_get_in_color_format(sii1136_t* self,
												sii1136_in_color_depth_t* input_color_depth,
												sii1136_video_range_exp_t* video_range_exp,
												sii1136_in_color_space_t* input_color_space);

sii1136_status_t sii1136_set_in_color_depth(sii1136_t* self,
											sii1136_in_color_depth_t input_color_depth);
sii1136_status_t sii1136_set_video_range_exp(sii1136_t* self,
												sii1136_video_range_exp_t video_range_exp);
sii1136_status_t sii1136_set_in_color_space(sii1136_t* self,
											sii1136_in_color_space_t input_color_space);
sii1136_status_t sii1136_set_in_color_format(sii1136_t* self,
												sii1136_in_color_depth_t input_color_depth,
												sii1136_video_range_exp_t video_range_exp,
												sii1136_in_color_space_t input_color_space);

/***** OUTPUT COLOR FORMAT REGISTERS *****/

sii1136_status_t sii1136_get_out_color_std(sii1136_t* self,
											sii1136_out_color_std_t* output_color_depth);
sii1136_status_t sii1136_get_video_rng_comp(sii1136_t* self,
											sii1136_video_rng_comp* video_range_compression);
sii1136_status_t sii1136_get_out_color_space(sii1136_t* self,
												sii1136_out_color_space_t* output_color_space);
sii1136_status_t sii1136_get_out_color_format(sii1136_t* self,
												sii1136_out_color_std_t* output_color_depth,
												sii1136_video_rng_comp* video_range_compression,
												sii1136_out_color_space_t* output_color_space);

sii1136_status_t sii1136_set_out_color_std(sii1136_t* self,
											sii1136_out_color_std_t output_color_depth);
sii1136_status_t sii1136_set_video_rng_comp(sii1136_t* self,
											sii1136_video_rng_comp video_range_compression);
sii1136_status_t sii1136_set_out_color_space(sii1136_t* self,
												sii1136_out_color_space_t output_color_space);
sii1136_status_t sii1136_set_out_color_format(sii1136_t* self,
												sii1136_out_color_std_t output_color_depth,
												sii1136_video_rng_comp video_range_compression,
												sii1136_out_color_space_t output_color_space);

/***** SYNC CONTROL REGISTERS *****/

sii1136_status_t sii1136_get_sync_method(sii1136_t* self, sii1136_sync_method_t* sync_method);
sii1136_status_t sii1136_get_yc_mux_enabled(sii1136_t* self, bool* yc_mux_enabled);
sii1136_status_t sii1136_get_f_bit_inverted(sii1136_t* self, bool* f_bit_inverted);
sii1136_status_t sii1136_get_de_adj_enabled(sii1136_t* self, bool* de_adj_enabled);
sii1136_status_t sii1136_get_vbit_adj_enabled(sii1136_t* self, bool* vbit_adj_enabled);
sii1136_status_t sii1136_get_vbit_adj_type(sii1136_t* self, sii1136_vbit_adj_type_t* vbit_adj_type);
sii1136_status_t sii1136_get_sync_controls(sii1136_t* self, sii1136_sync_method_t* sync_method,
											bool* yc_mux_enabled, bool* f_bit_inverted,
											bool* de_adj_enabled, bool* vbit_adj_enabled,
											sii1136_vbit_adj_type_t* vbit_adj_type);

sii1136_status_t sii1136_set_sync_method(sii1136_t* self, sii1136_sync_method_t sync_method);
sii1136_status_t sii1136_set_yc_mux_enabled(sii1136_t* self, bool yc_mux_enabled);
sii1136_status_t sii1136_set_f_bit_inverted(sii1136_t* self, bool f_bit_inverted);
sii1136_status_t sii1136_set_de_adj_enabled(sii1136_t* self, bool de_adj_enabled);
sii1136_status_t sii1136_set_vbit_adj_enabled(sii1136_t* self, bool vbit_adj_enabled);
sii1136_status_t sii1136_set_vbit_adj_type(sii1136_t* self, sii1136_vbit_adj_type_t vbit_adj_type);
sii1136_status_t sii1136_set_sync_controls(sii1136_t* self, sii1136_sync_method_t sync_method,
											bool yc_mux_enabled, bool f_bit_inverted,
											bool de_adj_enabled, bool vbit_adj_enabled,
											sii1136_vbit_adj_type_t vbit_adj_type);

/***** SYNC DETECTION REGISTERS (READ ONLY) *****/

sii1136_status_t sii1136_get_video_interlaced(sii1136_t* self, bool* video_interlaced);
sii1136_status_t sii1136_get_vsync_pol_det(sii1136_t* self,
											sii11136_sync_act_lvl_t* vsync_polarity);
sii1136_status_t sii1136_get_hsync_pol_det(sii1136_t* self,
											sii11136_sync_act_lvl_t* hsync_polarity);
sii1136_status_t sii1136_get_sync_detection(sii1136_t* self, bool* video_interlaced,
											sii11136_sync_act_lvl_t* vsync_polarity,
											sii11136_sync_act_lvl_t* hsync_polarity);

/***** YC INPUT REGISTERS *****/

sii1136_status_t sii1136_get_yc_msb_swapped(sii1136_t* self, bool* yc_msb_swapped);
sii1136_status_t sii1136_get_yc_ddr_bit(sii1136_t* self, sii1136_ddr_bits_t* ddr_bits);
sii1136_status_t sii1136_get_yc_nongap_enabled(sii1136_t* self, bool* non_gap_enabled);
sii1136_status_t sii1136_get_yc_input_mode(sii1136_t* self, bool* yc_input_mode);
sii1136_status_t sii1136_get_yc_in_format(sii1136_t* self, bool* yc_msb_swapped,
											sii1136_ddr_bits_t* ddr_bits, bool* non_gap_enabled,
											sii1136_yc_input_mode_t* yc_input_mode);

sii1136_status_t sii1136_set_yc_msb_swapped(sii1136_t* self, bool yc_msb_swapped);
sii1136_status_t sii1136_set_yc_ddr_bit(sii1136_t* self, sii1136_ddr_bits_t ddr_bits);
sii1136_status_t sii1136_set_yc_nongap_enabled(sii1136_t* self, bool non_gap_enabled);
sii1136_status_t sii1136_set_yc_input_mode(sii1136_t* self, bool yc_input_mode);
sii1136_status_t sii1136_set_yc_in_format(sii1136_t* self, bool yc_msb_swapped,
											sii1136_ddr_bits_t ddr_bits, bool non_gap_enabled,
											sii1136_yc_input_mode_t yc_input_mode);

/***** EXPLICIT DE GENERATOR REGISTERS *****/

sii1136_status_t sii1136_get_de_gen_enabled(sii1136_t* self, bool* de_gen_enabled);
sii1136_status_t sii1136_get_vsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t* vsync_polarity);
sii1136_status_t sii1136_get_hsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t* hsync_polarity);
sii1136_status_t sii1136_get_de_dly(sii1136_t* self, uint16_t* de_dly);
sii1136_status_t sii1136_get_de_top(sii1136_t* self, uint8_t* de_top);
sii1136_status_t sii1136_get_de_cnt(sii1136_t* self, uint16_t* de_cnt);
sii1136_status_t sii1136_get_de_lin(sii1136_t* self, uint16_t* de_lin);
sii1136_status_t sii1136_get_h_res_det(sii1136_t* self, uint16_t* horiz_resolution);
sii1136_status_t sii1136_get_v_res_det(sii1136_t* self, uint16_t* vert_resolution);
sii1136_status_t sii1136_get_de_gen_flags(sii1136_t* self, bool* de_gen_enabled,
											sii11136_sync_act_lvl_t* vsync_polarity,
											sii11136_sync_act_lvl_t* hsync_polarity);
sii1136_status_t sii1136_get_de_gen_meas(sii1136_t* self, uint16_t* de_dly, uint8_t* de_top,
											uint16_t* de_cnt, uint16_t* de_lin);
sii1136_status_t sii1136_get_det_res(sii1136_t* self, uint16_t* horiz_resolution,
										uint16_t* vert_resolution);

sii1136_status_t sii1136_set_de_gen_enabled(sii1136_t* self, bool de_gen_enabled);
sii1136_status_t sii1136_set_vsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t vsync_polarity);
sii1136_status_t sii1136_set_hsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t hsync_polarity);
sii1136_status_t sii1136_set_de_dly(sii1136_t* self, uint16_t de_dly);
sii1136_status_t sii1136_set_de_top(sii1136_t* self, uint8_t de_top);
sii1136_status_t sii1136_set_de_cnt(sii1136_t* self, uint16_t de_cnt);
sii1136_status_t sii1136_set_de_lin(sii1136_t* self, uint16_t de_lin);
sii1136_status_t sii1136_set_de_gen_flags(sii1136_t* self, bool de_gen_enabled,
											sii11136_sync_act_lvl_t vsync_polarity,
											sii11136_sync_act_lvl_t hsync_polarity);
sii1136_status_t sii1136_set_de_gen_meas(sii1136_t* self, uint16_t de_dly, uint8_t de_top,
											uint16_t de_cnt, uint16_t de_lin);

/***** EMBEDDED DE GENERATOR REGISTERS *****/

sii1136_status_t sii1136_get_emb_sync_enabled(sii1136_t* self, bool* embedded_sync_enabled);
sii1136_status_t sii1136_get_field2_offset(sii1136_t* self, uint16_t* field2_offset);
sii1136_status_t sii1136_get_hbit_to_hsync(sii1136_t* self, uint16_t* hbit_to_hsync);
sii1136_status_t sii1136_get_vbit_to_vsync(sii1136_t* self, uint8_t* vbit_to_vsync);
sii1136_status_t sii1136_get_hwidth(sii1136_t* self, uint16_t* hwidth);
sii1136_status_t sii1136_get_vwidth(sii1136_t* self, uint8_t* vwidth);
sii1136_status_t sii1136_get_emb_sync_regs(sii1136_t* self, bool* embedded_sync_enabled,
											uint16_t* field2_offset, uint16_t* hbit_to_hsync,
											uint8_t* vbit_to_vsync, uint16_t* hwidth,
											uint8_t* vwidth);

sii1136_status_t sii1136_set_emb_sync_enabled(sii1136_t* self, bool embedded_sync_enabled);
sii1136_status_t sii1136_set_field2_offset(sii1136_t* self, uint16_t field2_offset);
sii1136_status_t sii1136_set_hbit_to_hsync(sii1136_t* self, uint16_t hbit_to_hsync);
sii1136_status_t sii1136_set_vbit_to_vsync(sii1136_t* self, uint8_t vbit_to_vsync);
sii1136_status_t sii1136_set_hwidth(sii1136_t* self, uint16_t hwidth);
sii1136_status_t sii1136_set_vwidth(sii1136_t* self, uint8_t vwidth);
sii1136_status_t sii1136_set_emb_sync_regs(sii1136_t* self, bool embedded_sync_enabled,
											uint16_t field2_offset, uint16_t hbit_to_hsync,
											uint8_t vbit_to_vsync, uint16_t hwidth, uint8_t vwidth);

/***** TODO: AVI INFOFRAME REGISTERS *****/

/***** TODO: MISC INFOFRAME REGISTERS *****/

/***** SYSTEM CONTROL REGISTER *****/

sii1136_status_t sii1136_get_link_integrity_mode(sii1136_t* self,
													sii1136_link_int_mode_t* link_mode);
sii1136_status_t sii1136_get_tmds_output_control(sii1136_t* self,
													sii1136_tmds_out_cntl_t* tmds_control);
sii1136_status_t sii1136_get_av_muted(sii1136_t* self, bool* av_muted);
sii1136_status_t sii1136_get_ddc_bus_requested(sii1136_t* self, bool* ddc_bus_requested);
sii1136_status_t sii1136_get_ddc_bus_granted(sii1136_t* self, bool* bus_granted);
sii1136_status_t sii1136_get_output_mode(sii1136_t* self, sii1136_output_mode_t* output_mode);
sii1136_status_t sii1136_get_sys_cntl(sii1136_t* self, sii1136_link_int_mode_t* link_mode,
										sii1136_tmds_out_cntl_t* tmds_control, bool* av_muted,
										bool* ddc_bus_requested, bool* bus_granted,
										sii1136_output_mode_t* output_mode);

sii1136_status_t sii1136_set_link_integrity_mode(sii1136_t* self,
													sii1136_link_int_mode_t link_mode);
sii1136_status_t sii1136_set_tmds_output_control(sii1136_t* self,
													sii1136_tmds_out_cntl_t tmds_control);
sii1136_status_t sii1136_set_av_muted(sii1136_t* self, bool av_muted);
sii1136_status_t sii1136_set_ddc_bus_requested(sii1136_t* self, bool ddc_bus_requested);
sii1136_status_t sii1136_set_force_ddc_access(sii1136_t* self, bool force_ddc_access);
sii1136_status_t sii1136_set_output_mode(sii1136_t* self, sii1136_output_mode_t output_mode);
sii1136_status_t sii1136_set_sys_cntl(sii1136_t* self, sii1136_link_int_mode_t link_mode,
										sii1136_tmds_out_cntl_t tmds_control, bool av_muted,
										bool ddc_bus_requested, bool force_access,
										sii1136_output_mode_t output_mode);

/***** TODO: AUDIO CONFIGURATION REGISTERS *****/

/***** TODO: I2S CONFIGURATION REGISTERS *****/

/***** TODO: I2S MAPPING REGISTER *****/

/***** TODO: I2S STREAM HEADER REGISTER *****/

/***** INTERRUPT ENABLE REGISTER *****/

sii1136_status_t sii1136_get_auth_chg_int_en(sii1136_t* self, bool* auth_change_int_enabled);
sii1136_status_t sii1136_get_v_val_int_en(sii1136_t* self, bool* v_value_int_enabled);
sii1136_status_t sii1136_get_sec_chg_int_en(sii1136_t* self, bool* sec_chg_int_enabled);
sii1136_status_t sii1136_get_audio_err_int_en(sii1136_t* self, bool* audio_err_int_enabled);
sii1136_status_t sii1136_get_cpi_event_int_en(sii1136_t* self, bool* gpi_event_int_enabled);
sii1136_status_t sii1136_get_recv_sns_int_en(sii1136_t* self, bool* recv_sense_int_enabled);
sii1136_status_t sii1136_get_hot_plug_int_en(sii1136_t* self, bool* hot_plug_int_enabled);
sii1136_status_t sii1136_get_int_en_flags(sii1136_t* self, bool* auth_change_int_enabled,
											bool* v_value_int_enabled, bool* sec_chg_int_enabled,
											bool* audio_err_int_enabled,
											bool* gpi_event_int_enabled,
											bool* recv_sense_int_enabled,
											bool* hot_plug_int_enabled);

sii1136_status_t sii1136_set_auth_chg_int_en(sii1136_t* self, bool auth_change_int_enabled);
sii1136_status_t sii1136_set_v_val_int_en(sii1136_t* self, bool v_value_int_enabled);
sii1136_status_t sii1136_set_sec_chg_int_en(sii1136_t* self, bool sec_chg_int_enabled);
sii1136_status_t sii1136_set_audio_err_int_en(sii1136_t* self, bool audio_err_int_enabled);
sii1136_status_t sii1136_set_cpi_event_int_en(sii1136_t* self, bool gpi_event_int_enabled);
sii1136_status_t sii1136_set_recv_sns_int_en(sii1136_t* self, bool recv_sense_int_enabled);
sii1136_status_t sii1136_set_hot_plug_int_en(sii1136_t* self, bool hot_plug_int_enabled);
sii1136_status_t sii1136_set_int_en_flags(sii1136_t* self, bool auth_change_int_enabled,
											bool v_value_int_enabled, bool sec_chg_int_enabled,
											bool audio_err_int_enabled, bool gpi_event_int_enabled,
											bool recv_sense_int_enabled, bool hot_plug_int_enabled);

/***** INTERRUPT STATUS REGISTER *****/

sii1136_status_t sii1136_get_auth_chg_pending(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_sec_chg_pending(sii1136_t* self, bool* security_change_pending);
sii1136_status_t sii1136_get_audio_err_pending(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_rx_sns_detected(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_cpi_event_pending(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_hot_plug_state(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_ctrl_bus_pending(sii1136_t* self, bool* ctrl_bus_event_pending);
sii1136_status_t sii1136_get_rx_sns_event_pending(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_ctrl_bus_err_pending(sii1136_t* self, bool* auth_change_pending);
sii1136_status_t sii1136_get_conn_event_pending(sii1136_t* self, bool* auth_change_pending);

/***** POWER STATE REGISTER *****/

/***** TODO: SECURITY CONFIGURATION REGISTERS *****/

/***** TODO: AUXILLARY HDCP REGISTERS *****/
