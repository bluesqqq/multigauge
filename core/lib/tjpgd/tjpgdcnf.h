#pragma once

/* TJpgDec configuration for the core image decoder.
 *
 * The core decodes JPG assets to RGB888 and uses an 8192-byte work buffer,
 * so we keep the decoder settings aligned with that usage.
 */

#define JD_SZBUF 512
#define JD_FORMAT 0
#define JD_USE_SCALE 1
#define JD_FASTDECODE 1
#define JD_TBLCLIP 1
