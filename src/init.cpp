/*
 * This file is part of the libsigrokflow project.
 *
 * Copyright (C) 2018 Martin Ling <martin-sigrok@earth.li>
 * Copyright (C) 2018 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <gstreamermm.h>
#include <libsigrokflow/init.hpp>
#include <libsigrokflow/legacy_capture_device.hpp>
#include <libsigrokflow/legacy_input.hpp>
#include <libsigrokflow/legacy_output.hpp>
#include <libsigrokflow/legacy_decoder.hpp>

namespace Srf
{
using namespace std;

static bool srf_initialized_ = false;

void init()
{
	if (srf_initialized_)
		throw runtime_error("libsigrokflow is already initialized");

	if (!Gst::is_initialized())
		throw runtime_error("Gst::init() has not run yet");

#ifdef HAVE_LIBSIGROKCXX
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_capture_device",
			"Wrapper for capture devices using legacy libsigrok APIs",
			sigc::ptr_fun(&LegacyCaptureDevice::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "https://sigrok.org");
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_input",
			"Wrapper for inputs using legacy libsigrok APIs",
			sigc::ptr_fun(&LegacyInput::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "https://sigrok.org");
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_output",
			"Wrapper for outputs using legacy libsigrok APIs",
			sigc::ptr_fun(&LegacyOutput::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "https://sigrok.org");
#endif
#ifdef HAVE_LIBSIGROKDECODE
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_decoder",
			"Wrapper for protocol decoders using legacy libsigrokdecode APIs",
			sigc::ptr_fun(&LegacyDecoder::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "https://sigrok.org");
#endif

	srf_initialized_ = true;
}

void deinit()
{
	if (!srf_initialized_)
		throw runtime_error("libsigrokflow is not initialized");

	srf_initialized_ = false;
}

}
