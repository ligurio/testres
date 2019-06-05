/*
 * Copyright © 2018 Sergey Bronnikov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <math.h>

#include "parse_common.h"

const char *
format_string(enum test_format format)
{

	switch (format) {
	case FORMAT_TAP13:
		return "TAP13";
	case FORMAT_JUNIT:
		return "JUNIT";
	case FORMAT_SUBUNIT_V1:
		return "SUBUNIT_V1";
	case FORMAT_SUBUNIT_V2:
		return "SUBUNIT_V2";
	case FORMAT_UNKNOWN:
		return "UNKNOWN";
	default:
		return "UNKNOWN";
	}
}

const char *
format_status(enum test_status status)
{
	switch (status) {
	case STATUS_SUCCESS:
	case STATUS_PASS:
	case STATUS_OK:
		return "PASS";
	case STATUS_FAILED:
	case STATUS_FAILURE:
	case STATUS_NOTOK:
		return "FAIL";
	case STATUS_MISSING:
		return "MISSING";
	case STATUS_TODO:
		return "TODO";
	case STATUS_SKIPPED:
	case STATUS_SKIP:
		return "SKIP";
	case STATUS_UNDEFINED:
		return "UNDEFINED";
	case STATUS_ENUMERATION:
		return "ENUMERATION";
	case STATUS_INPROGRESS:
		return "INPROGRESS";
	case STATUS_UXSUCCESS:
		return "UXSUCCESS";
	case STATUS_XFAILURE:
		return "XFAILURE";
	case STATUS_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}
