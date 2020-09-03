#include "edid.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

static const uint8_t EDID_HEADER_OFFSET = 0x00;
static const uint8_t EDID_MFR_LSB_OFFSET = 0x08;
static const uint8_t EDID_MFR_MSB_OFFSET = 0x09;
static const uint8_t EDID_PROD_ID_LSB_OFFSET = 0x0A;
static const uint8_t EDID_PROD_ID_MSB_OFFSET = 0x0B;
static const uint8_t EDID_SER_NUM_BYTE_1_OFFSET = 0x0C;
static const uint8_t EDID_SER_NUM_BYTE_2_OFFSET = 0x0D;
static const uint8_t EDID_SER_NUM_BYTE_3_OFFSET = 0x0E;
static const uint8_t EDID_SER_NUM_BYTE_4_OFFSET = 0x0F;
static const uint8_t EDID_WEEK_OFFSET = 0x10;
static const uint8_t EDID_YEAR_OFFSET = 0x11;
static const uint8_t EDID_VERSION_OFFSET = 0x12;
static const uint8_t EDID_REVISION_OFFSET = 0x13;
static const uint8_t EDID_INPUT_DEF_OFFSET = 0x14;
static const uint8_t EDID_HORIZ_DIMENSION_OFFSET = 0x15;
static const uint8_t EDID_VERT_DIMENSION_OFFSET = 0x16;
static const uint8_t EDID_GAMMA_OFFSET = 0x17;
static const uint8_t EDID_FEAT_SUPPORT_OFFSET = 0x18;

static const uint16_t EDID_YEAR_ZERO = 1990;

edid_status_t edid_init(edid_t* edid, uint8_t* data) {
    if (edid == NULL || data == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    // Copy EDID file.
    memcpy((uint8_t*)edid->_data, data, EDID_BASIC_SIZE_B);

    return EDID_STATUS_OK;
}

edid_status_t edid_verify(edid_t* edid) {
    if (edid == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    // Ensure the EDID header is correct.
    static const uint8_t EDID_HEADER[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
    if (!memcmp(edid, EDID_HEADER, sizeof(EDID_HEADER))) {
        return EDID_STATUS_CORRUPT;
    }
    return EDID_STATUS_OK;
}

edid_status_t edid_get_mfr_code(edid_t* edid, uint8_t* code) {
    if (edid == NULL || code == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    // Manufacturer code uses compressed ASCII, where 0b00001 is A, 0b00010 is B, etc.
    // Actual ASCII characters start at 65. To convert compressed ASCII to standard, add this.
    static const uint8_t EDID_MFR_ASCII_OFFSET = 64;
    code[0] = (edid->_data[EDID_MFR_LSB_OFFSET] >> 2) + EDID_MFR_ASCII_OFFSET;
    code[1] = (edid->_data[EDID_MFR_LSB_OFFSET] & 0x03) +
              (edid->_data[EDID_MFR_MSB_OFFSET + 1] >> 5) + EDID_MFR_ASCII_OFFSET;
    code[2] = (edid->_data[EDID_MFR_MSB_OFFSET + 1] & 0x1F) + EDID_MFR_ASCII_OFFSET;

    // If any of the characters in the code are not valid uppercase letters.
    if (!isupper(code[0]) || !isupper(code[1]) || !isupper(code[2])) {
        return EDID_STATUS_REQ_FIELD_BLANK;
    }
    return EDID_STATUS_OK;
}

edid_status_t edid_get_prod_id(edid_t* edid, uint16_t* id) {
    if (edid == NULL || id == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    *id = edid->_data[EDID_PROD_ID_LSB_OFFSET] | (edid->_data[EDID_PROD_ID_MSB_OFFSET] << 8);

    return EDID_STATUS_OK;
}

edid_status_t edid_get_ser_num(edid_t* edid, uint32_t* ser_num) {
    if (edid == NULL || ser_num == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    *ser_num = edid->_data[EDID_SER_NUM_BYTE_1_OFFSET] |
               (edid->_data[EDID_SER_NUM_BYTE_2_OFFSET] << 8) |
               (edid->_data[EDID_SER_NUM_BYTE_3_OFFSET] << 16) |
               (edid->_data[EDID_SER_NUM_BYTE_4_OFFSET] << 24);

    if (*ser_num == 0) {
        return EDID_STATUS_OPT_FIELD_BLANK;
    }
    return EDID_STATUS_OK;
}

edid_status_t edid_get_date_type(edid_t* edid, edid_date_type_t* type) {
    if (edid == NULL || type == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t week_byte = edid->_data[EDID_WEEK_OFFSET];
    uint8_t year_byte = edid->_data[EDID_YEAR_OFFSET];

    // If date bytes contain any reserved values, the EDID is corrupt.
    if (year_byte <= 0x0F || (week_byte <= 0xFE && week_byte >= 0x37)) {
        return EDID_DATE_TYPE_CORRUPT;
    }

    if (week_byte == 0x00) {
        return EDID_DATE_TYPE_MFR_YEAR;
    } else if (week_byte == 0xFF) {
        return EDID_DATE_TYPE_MOD_YEAR;
    } else {
        return EDID_DATE_TYPE_MFR_YEAR_WEEK;
    }
}

edid_status_t edid_get_mfr_year_week(edid_t* edid, uint16_t* year, uint8_t* week) {
    if (edid == NULL || (year == NULL && week == NULL)) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    if (year != NULL) {
        *year = edid->_data[EDID_YEAR_OFFSET];
        // Check against reserved value.
        if (*year <= 0x0F) {
            return EDID_STATUS_CORRUPT;
        }
        // Epoch here is 1990; add that to get true date.
        *year += EDID_YEAR_ZERO;
    }

    if (week != NULL) {
        *week = edid->_data[EDID_WEEK_OFFSET];
        // Check against reserved values
        if (*week <= 0xFE && *week >= 0x37) {
            return EDID_STATUS_CORRUPT;
        }
        // Week is not available if the week byte is 0x00 or 0xFF.
        else if (*week == 0x00 || *week == 0xFF) {
            return EDID_STATUS_BAD_FIELD;
        }
    }

    return EDID_STATUS_OK;
}

edid_status_t edid_get_model_year(edid_t* edid, uint16_t* year) {
    if (edid == NULL || year == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    // If the week byte is not 0xFF, then the year byte does not hold model year.
    if (edid->_data[EDID_WEEK_OFFSET] != 0xFF) {
        return EDID_STATUS_BAD_FIELD;
    }

    *year = edid->_data[EDID_YEAR_OFFSET] + EDID_YEAR_ZERO;

    return EDID_STATUS_OK;
}

edid_status_t edid_get_edid_version(edid_t* edid, uint8_t* version) {
    if (edid == NULL || version == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    *version = edid->_data[EDID_VERSION_OFFSET];

    // Only 1.X and 2.X exist.
    if (*version == 0 || *version > 2) {
        return EDID_STATUS_CORRUPT;
    }
    return EDID_STATUS_OK;
}

edid_status_t edid_get_edid_revision(edid_t* edid, uint8_t* revision) {
    if (edid == NULL || revision == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    *revision = edid->_data[EDID_REVISION_OFFSET];

    // Only X.0 - X.4 exist.
    if (*revision > 4) {
        return EDID_STATUS_CORRUPT;
    }
    return EDID_STATUS_OK;
}

edid_status_t edid_get_edid_version_revision(edid_t* edid, uint8_t* version, uint8_t* revision) {
    if (edid == NULL || version == NULL || revision == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    *version = edid->_data[EDID_VERSION_OFFSET];
    *revision = edid->_data[EDID_REVISION_OFFSET];

    // Only versions 1.X and 2.X exist, only revisions X.0 - X.4 exist.
    if (*revision > 4 || *version == 0 || *version > 2) {
        return EDID_STATUS_CORRUPT;
    }
    return EDID_STATUS_OK;
}

edid_status_t edid_get_vid_sig_type(edid_t* edid, edid_vid_sig_type_t* type) {
    if (edid == NULL || type == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    *type = (edid->_data[EDID_INPUT_DEF_OFFSET] >> 7) & 1;

    return EDID_STATUS_OK;
}

edid_status_t edid_get_analog_input_def(edid_t* edid, edid_sig_lvl_std_t* sig_lvl_std,
                                        edid_blanking_lvl_t* blanking_lvl,
                                        edid_hvsync_seperate_t* hvsync_seperate,
                                        edid_comp_hsync_t* comp_hsync,
                                        edid_comp_grn_sync_t* comp_grn_sync,
                                        edid_vsync_serr_t* vsync_serr) {
    if (edid == NULL || (sig_lvl_std == NULL && blanking_lvl == NULL && hvsync_seperate == NULL &&
                         comp_hsync == NULL && comp_grn_sync == NULL && vsync_serr == NULL)) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t input_def_byte = edid->_data[EDID_INPUT_DEF_OFFSET];

    // Return error if this EDID describtes a digital interface.
    if (input_def_byte >> 7 == EDID_VID_SIG_TYPE_DIGITAL) {
        return EDID_STATUS_BAD_FIELD;
    }
    if (sig_lvl_std != NULL) {
        *sig_lvl_std = (input_def_byte >> 5) & 0x03;
    }
    if (blanking_lvl != NULL) {
        *blanking_lvl = (input_def_byte >> 4) & 0x01;
    }
    if (hvsync_seperate != NULL) {
        *hvsync_seperate = (input_def_byte >> 3) & 0x01;
    }
    if (comp_hsync != NULL) {
        *comp_hsync = (input_def_byte >> 2) & 0x01;
    }
    if (comp_grn_sync != NULL) {
        *comp_grn_sync = (input_def_byte >> 1) & 0x01;
    }
    if (vsync_serr != NULL) {
        *vsync_serr = input_def_byte & 0x01;
    }

    return EDID_STATUS_OK;
}

edid_status_t edid_get_digital_input_def(edid_t* edid, edid_color_depth_t* color_depth,
                                         edid_video_iface_t* video_iface) {
    if (edid == NULL || (color_depth == NULL && video_iface == NULL)) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t input_def_byte = edid->_data[EDID_INPUT_DEF_OFFSET];

    // Return error if this EDID describtes an analog interface.
    if (input_def_byte >> 7 == EDID_VID_SIG_TYPE_ANALOG) {
        return EDID_STATUS_BAD_FIELD;
    }
    if (color_depth != NULL) {
        *color_depth = (input_def_byte >> 4) & 0x07;
        if (*color_depth == 0x07) {
            return EDID_STATUS_CORRUPT;
        }
    }
    if (video_iface != NULL) {
        *video_iface = input_def_byte & 0x0F;
        if (*video_iface >= 0x06) {
            return EDID_STATUS_CORRUPT;
        }
    }

    return EDID_STATUS_OK;
}

edid_status_t edid_get_ratio_type(edid_t* edid, edid_ratio_type_t* ratio_type) {
    if (edid == NULL || ratio_type == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t horiz_byte = edid->_data[EDID_HORIZ_DIMENSION_OFFSET];
    uint8_t vert_byte = edid->_data[EDID_VERT_DIMENSION_OFFSET];

    if (horiz_byte == 0x00 && vert_byte == 0x00) {
        *ratio_type = EDID_RATIO_TYPE_UNDEFINED;
    } else if (horiz_byte == 0x00) {
        *ratio_type = EDID_RATIO_TYPE_PORTRAIT;
    } else if (vert_byte == 0x00) {
        *ratio_type = EDID_RATIO_TYPE_LANDSCAPE;
    } else {
        *ratio_type = EDID_RATIO_TYPE_SIZE;
    }

    return EDID_STATUS_OK;
}

edid_status_t edid_get_portrait_ratio(edid_t* edid, float* ratio) {
    if (edid == NULL || ratio == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t horiz_byte = edid->_data[EDID_HORIZ_DIMENSION_OFFSET];
    uint8_t vert_byte = edid->_data[EDID_VERT_DIMENSION_OFFSET];

    // Horizontal byte must be zero and vertical byte must contain data.
    if (horiz_byte != 0x00 || vert_byte == 0x00) {
        return EDID_STATUS_BAD_FIELD;
    }

    *ratio = 100 / ((float)vert_byte + 99);

    return EDID_STATUS_OK;
}

edid_status_t edid_get_landscape_ratio(edid_t* edid, float* ratio) {
    if (edid == NULL || ratio == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t horiz_byte = edid->_data[EDID_HORIZ_DIMENSION_OFFSET];
    uint8_t vert_byte = edid->_data[EDID_VERT_DIMENSION_OFFSET];

    // Vertical byte must be zero and horizontal byte must contain data.
    if (vert_byte != 0x00 || horiz_byte == 0x00) {
        return EDID_STATUS_BAD_FIELD;
    }

    *ratio = ((float)edid->_data[EDID_VERT_DIMENSION_OFFSET] + 99) / 100;

    return EDID_STATUS_OK;
}

edid_status_t edid_get_screen_size(edid_t* edid, uint8_t* horiz_size_cm, uint8_t* vert_size_cm) {
    if (edid == NULL || (horiz_size_cm == NULL && vert_size_cm == NULL)) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    uint8_t horiz_byte = edid->_data[EDID_HORIZ_DIMENSION_OFFSET];
    uint8_t vert_byte = edid->_data[EDID_VERT_DIMENSION_OFFSET];

    // Both bytes must contain nonzero dimensions; otherwise, these bytes describe ratio relatively.
    if (horiz_byte == 0x00 || horiz_byte == 0x00) {
        return EDID_STATUS_BAD_FIELD;
    }

    if (horiz_size_cm != NULL) {
        *horiz_size_cm = horiz_byte;
    }
    if (vert_size_cm != NULL) {
        *vert_size_cm = vert_byte;
    }

    return EDID_STATUS_OK;
}

edid_status_t edid_get_gamma(edid_t* edid, float* gamma) {
    if (edid == NULL || gamma == NULL) {
        return EDID_STATUS_NULL_ARG;
    } else if (edid->_data == NULL) {
        return EDID_STATUS_NULL_DATA;
    }

    // TODO: Fetch this from an extension block instead of returning error.
    if (edid->_data[EDID_GAMMA_OFFSET] == 0xFF) {
        return EDID_STATUS_REQ_FIELD_BLANK;
    }

    *gamma = ((float)edid->_data[EDID_GAMMA_OFFSET] + 100) / 100;

    return EDID_STATUS_OK;
}

int main() {
    edid_t my_edid;
    edid_init(&my_edid, edid_buf);
}
