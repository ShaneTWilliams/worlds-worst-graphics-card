#include "sii1136.h"

// I2C Statuses.
typedef enum {
	SII1136_I2C_STATUS_OK = 0x00,
	SII1136_I2C_STATUS_ERROR = 0x01,
	SII1136_I2C_STATUS_BUSY = 0x02,
	SII1136_I2C_STATUS_TIMEOUT = 0x03
} sii1136_i2c_status_t;

// SiI1136 Register Addresses.
enum {
	SII1136_REG_PXL_CLK_LSB = 0x00,
	SII1136_REG_PXL_CLK_MSB = 0x01,
	SII1136_REG_VFREQ_LSB = 0x02,
	SII1136_REG_VFREQ_MSB = 0x03,
	SII1136_REG_HORIZ_RES_LSB = 0x04,
	SII1136_REG_HORIZ_RES_MSB = 0x05,
	SII1136_REG_VERT_RES_LSB = 0x06,
	SII1136_REG_VERT_RES_MSB = 0x07,

	SII1136_REG_IN_VID_FMT = 0x08,
	SII1136_REG_IN_COLOR_FMT = 0x09,
	SII1136_REG_OUT_COLOR_FMT = 0x0A,
	SII1136_REG_YC_IN_FMT = 0x0B,

	SII1136_REG_SYS_CNTL = 0x1A,

	SII1136_REG_DEV_ID = 0x1B,
	SII1136_REG_DEV_REV_ID = 0x1C,
	SII1136_REG_TPI_REV = 0x1D,

	SII1136_REG_INT_EN = 0x3C,
	SII1136_REG_INT_STATUS = 0x3D,

	SII1136_REG_SYNC_GEN = 0x60,
	SII1136_REG_SYNC_DET = 0x61,

	SII1136_REG_DE_DLY_LSB = 0x62,
	SII1136_REG_DE_DLY_MSB = 0x63,
	SII1136_REG_DE_GEN_FLAGS = 0x63,
	SII1136_REG_DE_TOP = 0x64,
	SII1136_REG_DE_CNT_LSB = 0x66,
	SII1136_REG_DE_CNT_MSB = 0x67,
	SII1136_REG_DE_LIN_LSB = 0x68,
	SII1136_REG_DE_LIN_MSB = 0x69,
	SII1136_REG_H_RES_LSB = 0x6A,
	SII1136_REG_H_RES_MSB = 0x6B,
	SII1136_REG_V_RES_LSB = 0x6C,
	SII1136_REG_V_RES_MSB = 0x6D,

	SII1136_REG_HBIT_LSB = 0x62,
	SII1136_REG_HBIT_MSB = 0x63,
	SII1136_REG_EMB_SYNC_EN = 0x63,
	SII1136_REG_F2_OFST_LSB = 0x64,
	SII1136_REG_F2_OFST_MSB = 0x65,
	SII1136_REG_HWIDTH_LSB = 0x66,
	SII1136_REG_HWIDTH_MSB = 0x67,
	SII1136_REG_VBIT = 0x68,
	SII1136_REG_VWIDTH = 0x69,

	SII1136_REG_TPI_INIT = 0xC7
};

/********************************
 ***** I2C HELPER FUNCTIONS *****
 ********************************/

static inline sii1136_i2c_status_t sii1136_i2c_write_reg(sii1136_t* self, uint16_t mem_addr,
															uint8_t data) {
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(self->_i2c_handle, self->_i2c_addr, mem_addr, 1,
			&data, 1, self->_i2c_timeout);
	return (sii1136_i2c_status_t)status;
}

static inline sii1136_i2c_status_t sii1136_i2c_write_multi_reg(sii1136_t* self, uint16_t mem_addr,
																uint8_t* data, size_t size_b) {
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(self->_i2c_handle, self->_i2c_addr, mem_addr, 1,
			data, size_b, self->_i2c_timeout);
	return (sii1136_i2c_status_t)status;
}

static inline sii1136_i2c_status_t sii1136_i2c_read_reg(sii1136_t* self, uint16_t mem_addr,
														uint8_t* data) {
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(self->_i2c_handle, self->_i2c_addr, mem_addr, 1,
			data, 1, self->_i2c_timeout);
	return (sii1136_i2c_status_t)status;
}

static inline sii1136_i2c_status_t sii1136_i2c_read_multi_reg(sii1136_t* self, uint16_t mem_addr,
																uint8_t* data, size_t size_b) {
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(self->_i2c_handle, self->_i2c_addr, mem_addr, 1,
			data, size_b, self->_i2c_timeout);
	return (sii1136_i2c_status_t)status;
}

/****************************************
 ***** TPI INITIALIZATION FUNCTIONS *****
 ****************************************/

void sii1136_configure_i2c(sii1136_t* self, I2C_HandleTypeDef* i2c, uint8_t i2c_timeout,
							uint8_t i2c_addr) {
	self->_i2c_handle = i2c;
	self->_i2c_timeout = i2c_timeout;
	self->_i2c_addr = i2c_addr;
}

sii1136_status_t sii1136_init_tpi(sii1136_t* self) {
	sii1136_i2c_status_t status = sii1136_i2c_write_reg(self, SII1136_REG_TPI_INIT, 0x00);
	if (status == SII1136_I2C_STATUS_OK) {
		return SII1136_STATUS_OK;
	}
	return SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_tpi_ready(sii1136_t* self, sii1136_tpi_status_t* tpi_status) {
	if (self->_i2c_handle->State != HAL_I2C_STATE_READY) {
		*tpi_status = SII1136_TPI_STATUS_I2C_ERR;
		return SII1136_STATUS_OK;
	}
	uint8_t device_id = 0x00;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DEV_ID, &device_id);
	if (i2c_status != SII1136_I2C_STATUS_OK) {
		*tpi_status = SII1136_TPI_STATUS_I2C_ERR;
		return SII1136_STATUS_I2C_ERR;
	} else if (device_id != 0xB0) {
		*tpi_status = SII1136_TPI_STATUS_BAD_ID;
		return SII1136_STATUS_OK;
	}
	*tpi_status = SII1136_TPI_STATUS_READY;
	return SII1136_STATUS_OK;
}

/********************************************
 ***** REGISTER GETTER/SETTER FUNCTIONS *****
 ********************************************/

/***** IDENTIFICATION REGISTERS *****/

sii1136_status_t sii1136_get_device_id(sii1136_t* self, uint8_t* device_id) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DEV_ID, device_id);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_device_rev_id(sii1136_t* self, uint8_t* device_rev_id) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DEV_REV_ID,
			device_rev_id);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_tpi_rev(sii1136_t* self, uint8_t* tpi_revision) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_TPI_REV, tpi_revision);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_device_id(sii1136_t* self, uint8_t device_id) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_DEV_ID, device_id);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_device_rev_id(sii1136_t* self, uint8_t device_rev_id) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_DEV_REV_ID,
			device_rev_id);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_tpi_rev(sii1136_t* self, uint8_t tpi_revision) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_TPI_REV,
			tpi_revision);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** INPUT FORMAT REGISTERS *****/

sii1136_status_t sii1136_get_pixel_clock(sii1136_t* self, uint32_t* pixel_clock) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_PXL_CLK_LSB,
			(uint8_t*)pixel_clock, 2);
	*pixel_clock *= 10000;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vert_freq(sii1136_t* self, uint16_t* vert_freq) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_VFREQ_LSB,
			(uint8_t*)vert_freq, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_horiz_res(sii1136_t* self, uint16_t* horiz_res) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HORIZ_RES_LSB,
			(uint8_t*)horiz_res, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vert_res(sii1136_t* self, uint16_t* vert_res) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_VERT_RES_LSB,
			(uint8_t*)vert_res, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_pixel_clock(sii1136_t* self, uint32_t pixel_clock) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	pixel_clock /= 10000;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HORIZ_RES_LSB,
			(uint8_t*)&pixel_clock, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vert_freq(sii1136_t* self, uint16_t vert_freq) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_VFREQ_LSB,
			(uint8_t*)&vert_freq, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_horiz_res(sii1136_t* self, uint16_t horiz_res) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HORIZ_RES_LSB,
			(uint8_t*)&horiz_res, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vert_res(sii1136_t* self, uint16_t vert_res) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_VERT_RES_LSB,
			(uint8_t*)&vert_res, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** PIXEL CLOCK REGISTERS *****/

sii1136_status_t sii1136_get_tmds_clk_ratio(sii1136_t* self,
											sii1136_tmds_clk_ratio_t* tmds_clk_ratio) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT,
			tmds_clk_ratio);
	*tmds_clk_ratio >>= 6;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_bus_pxl_width(sii1136_t* self, sii1136_bus_pxl_width_t* bus_pxl_width) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT,
			bus_pxl_width);
	*bus_pxl_width = (*bus_pxl_width >> 5) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_video_clk_edge(sii1136_t* self,
											sii1136_video_clk_edge_t* video_clk_edge) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT,
			video_clk_edge);
	*video_clk_edge = (*video_clk_edge >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_pxl_repetition(sii1136_t* self,
											sii1136_pxl_repetition_t* pxl_repetition) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT,
			pxl_repetition);
	*pxl_repetition &= 0x07;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_input_format(sii1136_t* self, sii1136_tmds_clk_ratio_t* tmds_clk_ratio,
											sii1136_bus_pxl_width_t* bus_pxl_width,
											sii1136_video_clk_edge_t* video_clk_edge,
											sii1136_pxl_repetition_t* pxl_repetition) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT, &reg_val);
	*tmds_clk_ratio = reg_val >> 6;
	*bus_pxl_width = (reg_val >> 5) & 0x01;
	*video_clk_edge = (reg_val >> 4) & 0x01;
	*pxl_repetition = reg_val & 0x07;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_tmds_clk_ratio(sii1136_t* self,
											sii1136_tmds_clk_ratio_t tmds_clk_ratio) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT, &reg_val);
	reg_val &= ~(0x03 << 6);
	reg_val |= tmds_clk_ratio << 6;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_VID_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_bus_pxl_width(sii1136_t* self, sii1136_bus_pxl_width_t bus_pxl_width) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT, &reg_val);
	reg_val &= ~(0x01 << 5);
	reg_val |= bus_pxl_width << 5;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_VID_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_video_clk_edge(sii1136_t* self,
											sii1136_video_clk_edge_t video_clk_edge) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT, &reg_val);
	reg_val &= ~(0x01 << 4);
	reg_val |= video_clk_edge << 4;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_VID_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_pxl_repetition(sii1136_t* self,
											sii1136_pxl_repetition_t pxl_repetition) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_VID_FMT, &reg_val);
	reg_val &= 0xF0;
	reg_val |= pxl_repetition;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_VID_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_input_format(sii1136_t* self, sii1136_tmds_clk_ratio_t tmds_clk_ratio,
											sii1136_bus_pxl_width_t bus_pxl_width,
											sii1136_video_clk_edge_t video_clk_edge,
											sii1136_pxl_repetition_t pxl_repetition) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (tmds_clk_ratio << 6) | (bus_pxl_width << 5) | (video_clk_edge << 4)
			| pxl_repetition;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_IN_VID_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** INPUT COLOR FORMAT REGISTERS *****/

sii1136_status_t sii1136_get_in_color_depth(sii1136_t* self,
											sii1136_in_color_depth_t* input_color_depth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			input_color_depth);
	*input_color_depth >>= 6;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_video_range_exp(sii1136_t* self,
												sii1136_video_range_exp_t* video_range_exp) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			video_range_exp);
	*video_range_exp = (*video_range_exp >> 2) & 0x03;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_in_color_space(sii1136_t* self,
											sii1136_in_color_space_t* input_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			input_color_space);
	*input_color_space &= 0x03;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_in_color_format(sii1136_t* self,
												sii1136_in_color_depth_t* input_color_depth,
												sii1136_video_range_exp_t* video_range_exp,
												sii1136_in_color_space_t* input_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			&reg_val);
	*input_color_depth = reg_val >> 6;
	*video_range_exp = (reg_val >> 2) & 0x03;
	*input_color_space = reg_val & 0x03;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_in_color_depth(sii1136_t* self,
											sii1136_in_color_depth_t input_color_depth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			&reg_val);
	reg_val &= ~(0x03 << 6);
	reg_val |= (input_color_depth << 6);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_COLOR_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_video_range_exp(sii1136_t* self,
												sii1136_video_range_exp_t video_range_exp) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			&reg_val);
	reg_val &= ~(0x03 << 2);
	reg_val |= (video_range_exp << 2);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_COLOR_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_in_color_space(sii1136_t* self,
											sii1136_in_color_space_t input_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_IN_COLOR_FMT,
			&reg_val);
	reg_val &= ~0x03;
	reg_val |= input_color_space;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_IN_COLOR_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_in_color_format(sii1136_t* self,
												sii1136_in_color_depth_t input_color_depth,
												sii1136_video_range_exp_t video_range_exp,
												sii1136_in_color_space_t input_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (input_color_depth << 6) | (video_range_exp << 2) | input_color_space;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_IN_COLOR_FMT,
			reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** OUTPUT COLOR FORMAT REGISTERS *****/

sii1136_status_t sii1136_get_out_color_std(sii1136_t* self,
											sii1136_out_color_std_t* output_color_depth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			output_color_depth);
	*output_color_depth = (*output_color_depth >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_video_rng_comp(sii1136_t* self,
											sii1136_video_rng_comp* video_range_compression) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			video_range_compression);
	*video_range_compression = (*video_range_compression >> 2) & 0x03;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_out_color_space(sii1136_t* self,
												sii1136_out_color_space_t* output_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			output_color_space);
	// 0x03 and 0x00 both represent RGB: change 0x03 to 0x00.
	*output_color_space =
			*output_color_space == 0x03 ? SII1136_IN_COLOR_SPACE_RGB : *output_color_space & 0x03;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_out_color_format(sii1136_t* self,
												sii1136_out_color_std_t* output_color_depth,
												sii1136_video_rng_comp* video_range_compression,
												sii1136_out_color_space_t* output_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			&reg_val);
	*output_color_depth = (reg_val >> 4) & 0x01;
	*video_range_compression = (reg_val >> 2) & 0x03;
	// 0x03 and 0x00 both represent RGB: change 0x03 to 0x00.
	*output_color_space = reg_val == 0x03 ? SII1136_IN_COLOR_SPACE_RGB : reg_val & 0x03;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_out_color_std(sii1136_t* self,
											sii1136_out_color_std_t output_color_depth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			&reg_val);
	reg_val &= ~(0x01 << 4);
	reg_val |= (output_color_depth << 4);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_OUT_COLOR_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_video_rng_comp(sii1136_t* self,
											sii1136_video_rng_comp video_range_compression) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			&reg_val);
	reg_val &= ~(0x03 << 2);
	reg_val |= (video_range_compression << 2);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_OUT_COLOR_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_out_color_space(sii1136_t* self,
												sii1136_out_color_space_t output_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_OUT_COLOR_FMT,
			&reg_val);
	reg_val &= ~0x03;
	reg_val |= output_color_space;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_OUT_COLOR_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_out_color_format(sii1136_t* self,
												sii1136_out_color_std_t output_color_depth,
												sii1136_video_rng_comp video_range_compression,
												sii1136_out_color_space_t output_color_space) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (output_color_depth << 4) | (video_range_compression << 2)
			| output_color_space;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_OUT_COLOR_FMT,
			reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** SYNC CONTROL REGISTERS *****/

sii1136_status_t sii1136_get_sync_method(sii1136_t* self, sii1136_sync_method_t* sync_method) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, sync_method);
	*sync_method >>= 7;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_yc_mux_enabled(sii1136_t* self, bool* yc_mux_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN,
			(uint8_t*)yc_mux_enabled);
	*yc_mux_enabled = (*yc_mux_enabled >> 5) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_f_bit_inverted(sii1136_t* self, bool* f_bit_inverted) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN,
			(uint8_t*)f_bit_inverted);
	*f_bit_inverted = (*f_bit_inverted >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_adj_enabled(sii1136_t* self, bool* de_adj_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN,
			(uint8_t*)de_adj_enabled);
	*de_adj_enabled = (*de_adj_enabled >> 2) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vbit_adj_enabled(sii1136_t* self, bool* vbit_adj_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN,
			(uint8_t*)vbit_adj_enabled);
	*vbit_adj_enabled = (*vbit_adj_enabled >> 1) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vbit_adj_type(sii1136_t* self, sii1136_vbit_adj_type_t* vbit_adj_type) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN,
			vbit_adj_type);
	*vbit_adj_type &= 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_sync_controls(sii1136_t* self, sii1136_sync_method_t* sync_method,
bool* yc_mux_enabled,
											bool* f_bit_inverted,
											bool* de_adj_enabled,
											bool* vbit_adj_enabled,
											sii1136_vbit_adj_type_t* vbit_adj_type) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	*sync_method = reg_val >> 7;
	*yc_mux_enabled = (reg_val >> 5) & 0x01;
	*f_bit_inverted = (reg_val >> 4) & 0x01;
	*de_adj_enabled = (reg_val >> 2) & 0x01;
	*vbit_adj_enabled = (reg_val >> 1) & 0x01;
	*vbit_adj_type = reg_val & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_sync_method(sii1136_t* self, sii1136_sync_method_t sync_method) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	reg_val &= ~(0x01 << 7);
	reg_val |= (sync_method << 7);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_yc_mux_enabled(sii1136_t* self, bool yc_mux_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	reg_val &= ~(0x01 << 5);
	reg_val |= (yc_mux_enabled << 5);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_f_bit_inverted(sii1136_t* self, bool f_bit_inverted) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	reg_val &= ~(0x01 << 4);
	reg_val |= (f_bit_inverted << 4);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_adj_enabled(sii1136_t* self, bool de_adj_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	reg_val &= ~(0x01 << 2);
	reg_val |= (de_adj_enabled << 2);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vbit_adj_enabled(sii1136_t* self, bool vbit_adj_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	reg_val &= ~(0x01 << 1);
	reg_val |= (vbit_adj_enabled << 1);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vbit_adj_type(sii1136_t* self, sii1136_vbit_adj_type_t vbit_adj_type) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_GEN, &reg_val);
	reg_val &= ~0x01;
	reg_val |= vbit_adj_type;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_sync_controls(sii1136_t* self, sii1136_sync_method_t sync_method,
bool yc_mux_enabled,
											bool f_bit_inverted,
											bool de_adj_enabled,
											bool vbit_adj_enabled,
											sii1136_vbit_adj_type_t vbit_adj_type) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (sync_method << 7) | (yc_mux_enabled << 5) | (f_bit_inverted << 4)
			| (de_adj_enabled << 2) | (vbit_adj_enabled << 1) | vbit_adj_type;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_SYNC_GEN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** SYNC DETECTION REGISTERS (READ ONLY) *****/

sii1136_status_t sii1136_get_video_interlaced(sii1136_t* self, bool* video_interlaced) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_DET,
			(uint8_t*)video_interlaced);
	*video_interlaced = (*video_interlaced >> 2) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vsync_polarity(sii1136_t* self,
											sii11136_sync_act_lvl_t* vsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_DET,
			vsync_polarity);
	*vsync_polarity = (*vsync_polarity >> 1) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_hsync_polarity(sii1136_t* self,
											sii11136_sync_act_lvl_t* hsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_DET,
			hsync_polarity);
	*hsync_polarity = *hsync_polarity & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_sync_detection(sii1136_t* self, bool* video_interlaced,
											sii11136_sync_act_lvl_t* vsync_polarity,
											sii11136_sync_act_lvl_t* hsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYNC_DET, &reg_val);
	*video_interlaced = (reg_val >> 2) & 0x01;
	*vsync_polarity = (reg_val >> 1) & 0x01;
	*hsync_polarity = reg_val & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** YC INPUT REGISTERS *****/

sii1136_status_t sii1136_get_yc_msb_swapped(sii1136_t* self, bool* yc_msb_swapped) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT,
			(uint8_t*)yc_msb_swapped);
	*yc_msb_swapped >>= 7;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_yc_ddr_bit(sii1136_t* self, sii1136_ddr_bits_t* ddr_bits) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT,
			(uint8_t*)ddr_bits);
	*ddr_bits = (*ddr_bits >> 6) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_yc_nongap_enabled(sii1136_t* self, bool* non_gap_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT,
			(uint8_t*)non_gap_enabled);
	*non_gap_enabled = (*non_gap_enabled >> 3) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_yc_input_mode(sii1136_t* self, bool* yc_input_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT,
			(uint8_t*)yc_input_mode);
	*yc_input_mode &= 0x07;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_yc_in_format(sii1136_t* self, bool* yc_msb_swapped,
											sii1136_ddr_bits_t* ddr_bits, bool* non_gap_enabled,
											sii1136_yc_input_mode_t* yc_input_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT, &reg_val);
	*yc_msb_swapped = reg_val >> 7;
	*ddr_bits = (reg_val >> 6) & 0x01;
	*non_gap_enabled = (reg_val >> 3) & 0x01;
	*yc_input_mode = reg_val & 0x07;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_yc_msb_swapped(sii1136_t* self, bool yc_msb_swapped) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT, &reg_val);
	reg_val &= ~(0x01 << 7);
	reg_val |= (yc_msb_swapped << 7);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_YC_IN_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_yc_ddr_bit(sii1136_t* self, sii1136_ddr_bits_t ddr_bits) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT, &reg_val);
	reg_val &= ~(0x01 << 6);
	reg_val |= (ddr_bits << 6);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_YC_IN_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_yc_nongap_enabled(sii1136_t* self, bool non_gap_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT, &reg_val);
	reg_val &= ~(0x01 << 3);
	reg_val |= (non_gap_enabled << 3);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_YC_IN_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_yc_input_mode(sii1136_t* self, bool yc_input_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_YC_IN_FMT, &reg_val);
	reg_val &= ~0x07;
	reg_val |= yc_input_mode;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_YC_IN_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_yc_in_format(sii1136_t* self, bool yc_msb_swapped,
											sii1136_ddr_bits_t ddr_bits, bool non_gap_enabled,
											sii1136_yc_input_mode_t yc_input_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (yc_msb_swapped << 7) | (ddr_bits << 6) | (non_gap_enabled << 3)
			| yc_input_mode;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_YC_IN_FMT, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** EXPLICIT DE GENERATOR REGISTERS *****/

sii1136_status_t sii1136_get_de_gen_enabled(sii1136_t* self, bool* de_gen_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			(uint8_t*)de_gen_enabled);
	*de_gen_enabled >>= 6;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t* vsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			(uint8_t*)vsync_polarity);
	*vsync_polarity = (*vsync_polarity >> 5) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_hsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t* hsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			(uint8_t*)hsync_polarity);
	*hsync_polarity = (*hsync_polarity >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_dly(sii1136_t* self, uint16_t* de_dly) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_DE_DLY_LSB,
			(uint8_t*)de_dly, 2);
	*de_dly &= 0x03FF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_top(sii1136_t* self, uint8_t* de_top) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_TOP,
			(uint8_t*)de_top);
	*de_top &= 0x7F;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_cnt(sii1136_t* self, uint16_t* de_cnt) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_DE_CNT_LSB,
			(uint8_t*)de_cnt, 2);
	*de_cnt &= 0x0FFF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_lin(sii1136_t* self, uint16_t* de_lin) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_DE_LIN_LSB,
			(uint8_t*)de_lin, 2);
	*de_lin &= 0x07FF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_h_res_det(sii1136_t* self, uint16_t* horiz_resolution) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HORIZ_RES_LSB,
			(uint8_t*)horiz_resolution, 2);
	*horiz_resolution &= 0x0FFF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_v_res_det(sii1136_t* self, uint16_t* vert_resolution) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_VERT_RES_LSB,
			(uint8_t*)vert_resolution, 2);
	*vert_resolution &= 0x0FFF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_gen_flags(sii1136_t* self, bool* de_gen_enabled,
											sii11136_sync_act_lvl_t* vsync_polarity,
											sii11136_sync_act_lvl_t* hsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&reg_val);
	*de_gen_enabled = (reg_val >> 6) & 0x01;
	*vsync_polarity = (reg_val >> 5) & 0x01;
	*hsync_polarity = (reg_val >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_de_gen_meas(sii1136_t* self, uint16_t* de_dly, uint8_t* de_top,
											uint16_t* de_cnt, uint16_t* de_lin) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint16_t reg_buf[4];
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_DE_DLY_LSB,
			(uint8_t*)reg_buf, sizeof(reg_buf));
	*de_dly = reg_buf[0] & 0x03FF;
	*de_top = reg_buf[1] & 0x007F;
	*de_cnt = reg_buf[2] & 0x0FFF;
	*de_lin = reg_buf[3] & 0x07FF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_det_res(sii1136_t* self, uint16_t* horiz_resolution,
										uint16_t* vert_resolution) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint16_t reg_buf[2];
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HORIZ_RES_LSB,
			(uint8_t*)reg_buf, sizeof(reg_buf));
	*horiz_resolution = reg_buf[0] & 0x0FFF;
	*vert_resolution = reg_buf[1] & 0x0FFF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_gen_enabled(sii1136_t* self, bool de_gen_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&reg_val);
	reg_val &= ~(0x07 << 4);
	reg_val |= (de_gen_enabled << 6);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_DE_GEN_FLAGS, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t vsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&reg_val);
	reg_val &= ~(0x07 << 4);
	reg_val |= (vsync_polarity << 5);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_DE_GEN_FLAGS, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_hsync_pol_de_gen(sii1136_t* self,
												sii11136_sync_act_lvl_t hsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&reg_val);
	reg_val &= ~(0x07 << 4);
	reg_val |= (hsync_polarity << 4);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_DE_GEN_FLAGS, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_dly(sii1136_t* self, uint16_t de_dly) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&reg_val);
	uint8_t de_gen_flags = (reg_val >> 4) & 0x07;
	de_dly |= de_gen_flags << 12;
	i2c_status |= sii1136_i2c_write_multi_reg(self, SII1136_REG_DE_DLY_LSB, (uint8_t*)&de_dly, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_top(sii1136_t* self, uint8_t de_top) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_DE_DLY_LSB, de_top);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_cnt(sii1136_t* self, uint16_t de_cnt) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_multi_reg(self, SII1136_REG_DE_DLY_LSB,
			(uint8_t*)&de_cnt, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_lin(sii1136_t* self, uint16_t de_lin) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_multi_reg(self, SII1136_REG_DE_DLY_LSB,
			(uint8_t*)&de_lin, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_gen_flags(sii1136_t* self, bool de_gen_enabled,
											sii11136_sync_act_lvl_t vsync_polarity,
											sii11136_sync_act_lvl_t hsync_polarity) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&reg_val);
	reg_val &= 0x03;
	reg_val |= (de_gen_enabled << 6) | (vsync_polarity << 5) | (hsync_polarity << 4);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_DE_GEN_FLAGS, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_de_gen_meas(sii1136_t* self, uint16_t de_dly, uint8_t de_top,
											uint16_t de_cnt, uint16_t de_lin) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t de_gen_flags;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_DE_GEN_FLAGS,
			&de_gen_flags);
	de_gen_flags &= 0xF0;
	uint16_t reg_buf[4] = { de_dly | (de_gen_flags << 8), de_top, de_cnt, de_lin };
	i2c_status |= sii1136_i2c_write_multi_reg(self, SII1136_REG_DE_DLY_LSB, (uint8_t*)reg_buf, 8);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** EMBEDDED DE GENERATOR REGISTERS *****/

sii1136_status_t sii1136_get_emb_sync_enabled(sii1136_t* self, bool* embedded_sync_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_EMB_SYNC_EN,
			(uint8_t*)embedded_sync_enabled);
	*embedded_sync_enabled >>= 6;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_field2_offset(sii1136_t* self, uint16_t* field2_offset) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_F2_OFST_LSB,
			(uint8_t*)field2_offset, 2);
	*field2_offset &= 0x1FFF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_hbit_to_hsync(sii1136_t* self, uint16_t* hbit_to_hsync) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HBIT_LSB,
			(uint8_t*)hbit_to_hsync, 2);
	*hbit_to_hsync &= 0x03FF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vbit_to_vsync(sii1136_t* self, uint8_t* vbit_to_vsync) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_VBIT, vbit_to_vsync);
	*vbit_to_vsync &= 0x3F;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_hwidth(sii1136_t* self, uint16_t* hwidth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HWIDTH_LSB,
			(uint8_t*)hwidth, 2);
	*hwidth &= 0x03FF;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_vwidth(sii1136_t* self, uint8_t* vwidth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_VWIDTH, vwidth);
	*vwidth &= 0x3F;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_emb_sync_regs(sii1136_t* self, bool* embedded_sync_enabled,
											uint16_t* field2_offset, uint16_t* hbit_to_hsync,
											uint8_t* vbit_to_vsync, uint16_t* hwidth,
											uint8_t* vwidth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_buf[8];
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_multi_reg(self, SII1136_REG_HBIT_LSB,
			(uint8_t*)reg_buf, sizeof(reg_buf));
	*embedded_sync_enabled = (reg_buf[1] >> 6) & 0x01;
	*field2_offset = reg_buf[2] | ((reg_buf[3] & 0x1F) << 8);
	*hbit_to_hsync = reg_buf[0] | ((reg_buf[1] & 0x03) << 8);
	*vbit_to_vsync = reg_buf[6] & 0x3F;
	*hwidth = reg_buf[4] | ((reg_buf[5] & 0x03) << 8);
	*vwidth = reg_buf[7] & 0x3F;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_emb_sync_enabled(sii1136_t* self, bool embedded_sync_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_EMB_SYNC_EN, &reg_val);
	reg_val &= ~(0x01 << 6);
	reg_val |= (embedded_sync_enabled << 6);
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_EMB_SYNC_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_field2_offset(sii1136_t* self, uint16_t field2_offset) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_multi_reg(self, SII1136_REG_F2_OFST_LSB,
			(uint8_t*)&field2_offset, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_hbit_to_hsync(sii1136_t* self, uint16_t hbit_to_hsync) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_EMB_SYNC_EN, &reg_val);
	hbit_to_hsync |= ((reg_val >> 6) & 0x01) << 14;
	i2c_status |= sii1136_i2c_write_multi_reg(self, SII1136_REG_HBIT_LSB, (uint8_t*)&hbit_to_hsync,
			2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vbit_to_vsync(sii1136_t* self, uint8_t vbit_to_vsync) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_VBIT, vbit_to_vsync);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_hwidth(sii1136_t* self, uint16_t hwidth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_multi_reg(self, SII1136_REG_HWIDTH_LSB,
			(uint8_t*)&hwidth, 2);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_vwidth(sii1136_t* self, uint8_t vwidth) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_VWIDTH, vwidth);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_emb_sync_regs(sii1136_t* self, bool embedded_sync_enabled,
											uint16_t field2_offset, uint16_t hbit_to_hsync,
											uint8_t vbit_to_vsync, uint16_t hwidth, uint8_t vwidth) {
	uint8_t reg_buf[8];
	reg_buf[0] = hbit_to_hsync;
	reg_buf[1] = 0x00;
	reg_buf[1] = (hbit_to_hsync >> 8) | (embedded_sync_enabled << 6);
	reg_buf[2] = field2_offset;
	reg_buf[3] = field2_offset >> 8;
	reg_buf[4] = hwidth;
	reg_buf[5] = hwidth >> 8;
	reg_buf[6] = vbit_to_vsync;
	reg_buf[7] = vwidth;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_multi_reg(self, SII1136_REG_HBIT_LSB,
			reg_buf, sizeof(reg_buf));
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** SYSTEM CONTROL REGISTER *****/

sii1136_status_t sii1136_get_link_integrity_mode(sii1136_t* self,
													sii1136_link_int_mode_t* link_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, link_mode);
	*link_mode = (*link_mode >> 6) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_tmds_output_control(sii1136_t* self,
													sii1136_tmds_out_cntl_t* tmds_control) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL,
			tmds_control);
	*tmds_control = (*tmds_control >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_av_muted(sii1136_t* self, bool* av_muted) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL,
			(uint8_t*)av_muted);
	*av_muted = (*av_muted >> 3) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_ddc_bus_requested(sii1136_t* self, bool* ddc_bus_requested) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL,
			(uint8_t*)ddc_bus_requested);
	*ddc_bus_requested = (*ddc_bus_requested >> 2) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_ddc_bus_granted(sii1136_t* self, bool* bus_granted) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL,
			(uint8_t*)bus_granted);
	*bus_granted = (*bus_granted >> 1) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_output_mode(sii1136_t* self, sii1136_output_mode_t* output_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, output_mode);
	*output_mode &= 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_sys_cntl(sii1136_t* self, sii1136_link_int_mode_t* link_mode,
										sii1136_tmds_out_cntl_t* tmds_control, bool* av_muted,
										bool* ddc_bus_requested,
										bool* bus_granted, sii1136_output_mode_t* output_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	*link_mode = (*link_mode >> 6) & 0x01;
	*tmds_control = (*tmds_control >> 4) & 0x01;
	*av_muted = (*av_muted >> 3) & 0x01;
	*ddc_bus_requested = (*ddc_bus_requested >> 2) & 0x01;
	*bus_granted = (*bus_granted >> 1) & 0x01;
	*output_mode &= 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_link_integrity_mode(sii1136_t* self, sii1136_link_int_mode_t link_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	reg_val &= ~(1 << 6);
	reg_val |= link_mode << 6;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_tmds_output_control(sii1136_t* self,
													sii1136_tmds_out_cntl_t tmds_control) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	reg_val &= ~(0x01 << 4);
	reg_val |= tmds_control << 4;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_av_muted(sii1136_t* self, bool av_muted) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	reg_val &= ~(0x01 << 3);
	reg_val |= av_muted << 3;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_ddc_bus_requested(sii1136_t* self, bool ddc_bus_requested) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	reg_val &= ~(0x01 << 2);
	reg_val |= ddc_bus_requested << 2;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_force_ddc_access(sii1136_t* self, bool force_ddc_access) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	reg_val &= ~(0x01 << 1);
	reg_val |= force_ddc_access << 1;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_output_mode(sii1136_t* self, sii1136_output_mode_t output_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_SYS_CNTL, &reg_val);
	reg_val &= ~0x01;
	reg_val |= output_mode;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_sys_cntl(sii1136_t* self, sii1136_link_int_mode_t link_mode,
										sii1136_tmds_out_cntl_t tmds_control, bool av_muted,
										bool ddc_bus_requested,
										bool force_access, sii1136_output_mode_t output_mode) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (link_mode << 6) | (tmds_control << 4) | (av_muted << 3)
			| (ddc_bus_requested << 2) | (force_access << 1) | output_mode;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_SYS_CNTL, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

/***** INTERRUPT ENABLE REGISTER *****/

sii1136_status_t sii1136_get_auth_chg_int_en(sii1136_t* self, bool* auth_change_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)auth_change_int_enabled);
	*auth_change_int_enabled >>= 7;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_v_val_int_en(sii1136_t* self, bool* v_value_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)v_value_int_enabled);
	*v_value_int_enabled = (*v_value_int_enabled >> 6) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_sec_chg_int_en(sii1136_t* self, bool* sec_chg_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)sec_chg_int_enabled);
	*sec_chg_int_enabled = (*sec_chg_int_enabled >> 5) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_audio_err_int_en(sii1136_t* self, bool* audio_err_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)audio_err_int_enabled);
	*audio_err_int_enabled = (*audio_err_int_enabled >> 4) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_cpi_event_int_en(sii1136_t* self, bool* gpi_event_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)gpi_event_int_enabled);
	*gpi_event_int_enabled = (*gpi_event_int_enabled >> 3) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_recv_sns_int_en(sii1136_t* self, bool* recv_sense_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)recv_sense_int_enabled);
	*recv_sense_int_enabled = (*recv_sense_int_enabled >> 1) & 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_hot_plug_int_en(sii1136_t* self, bool* hot_plug_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)hot_plug_int_enabled);
	*hot_plug_int_enabled &= 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_get_int_en_flags(sii1136_t* self, bool* auth_change_int_enabled,
bool* v_value_int_enabled,
											bool* sec_chg_int_enabled,
											bool* audio_err_int_enabled,
											bool* gpi_event_int_enabled,
											bool* recv_sense_int_enabled,
											bool* hot_plug_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN,
			(uint8_t*)auth_change_int_enabled);
	*auth_change_int_enabled >>= 7;
	*v_value_int_enabled = (*v_value_int_enabled >> 6) & 0x01;
	*sec_chg_int_enabled = (*sec_chg_int_enabled >> 5) & 0x01;
	*audio_err_int_enabled = (*audio_err_int_enabled >> 4) & 0x01;
	*gpi_event_int_enabled = (*gpi_event_int_enabled >> 3) & 0x01;
	*recv_sense_int_enabled = (*recv_sense_int_enabled >> 1) & 0x01;
	*hot_plug_int_enabled &= 0x01;
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_auth_chg_int_en(sii1136_t* self, bool auth_change_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~(0x01 << 7);
	reg_val |= auth_change_int_enabled << 7;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_v_val_int_en(sii1136_t* self, bool v_value_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~(0x01 << 6);
	reg_val |= v_value_int_enabled << 6;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_sec_chg_int_en(sii1136_t* self, bool sec_chg_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~(0x01 << 5);
	reg_val |= sec_chg_int_enabled << 5;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_audio_err_int_en(sii1136_t* self, bool audio_err_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~(0x01 << 4);
	reg_val |= audio_err_int_enabled << 4;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_cpi_event_int_en(sii1136_t* self, bool gpi_event_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~(0x01 << 3);
	reg_val |= gpi_event_int_enabled << 3;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_recv_sns_int_en(sii1136_t* self, bool recv_sense_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~(0x01 << 1);
	reg_val |= recv_sense_int_enabled << 1;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_hot_plug_int_en(sii1136_t* self, bool hot_plug_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_read_reg(self, SII1136_REG_INT_EN, &reg_val);
	reg_val &= ~0x01;
	reg_val |= hot_plug_int_enabled;
	i2c_status |= sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

sii1136_status_t sii1136_set_int_en_flags(sii1136_t* self, bool auth_change_int_enabled,
bool v_value_int_enabled,
											bool sec_chg_int_enabled,
											bool audio_err_int_enabled,
											bool gpi_event_int_enabled,
											bool recv_sense_int_enabled,
											bool hot_plug_int_enabled) {
	if (self == NULL) {
		return SII1136_STATUS_NULL_ARG;
	}
	uint8_t reg_val = (auth_change_int_enabled << 7) | (v_value_int_enabled << 6)
			| (sec_chg_int_enabled << 5) | (audio_err_int_enabled << 4)
			| (gpi_event_int_enabled << 3) | (recv_sense_int_enabled << 1) | hot_plug_int_enabled;
	sii1136_i2c_status_t i2c_status = sii1136_i2c_write_reg(self, SII1136_REG_INT_EN, reg_val);
	return i2c_status == SII1136_I2C_STATUS_OK ? SII1136_STATUS_OK : SII1136_STATUS_I2C_ERR;
}

