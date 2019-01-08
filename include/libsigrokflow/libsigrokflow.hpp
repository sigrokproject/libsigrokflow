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

#ifndef LIBSIGROKFLOW_LIBSIGROKFLOW_HPP
#define LIBSIGROKFLOW_LIBSIGROKFLOW_HPP

/* Temporary workaround, will be dropped later. */
#define HAVE_LIBSIGROKCXX 1
#define HAVE_LIBSIGROKDECODE 1

#include <gstreamermm.h>
#include <gstreamermm/private/element_p.h>
#include <gstreamermm/private/basesink_p.h>
#ifdef HAVE_LIBSIGROKCXX
#include <libsigrokcxx/libsigrokcxx.hpp>
#endif
#ifdef HAVE_LIBSIGROKDECODE
#include <libsigrokdecode/libsigrokdecode.h>
#endif

namespace Srf
{

using namespace std;

void init();

class Block
{
        /* Config API etc goes here */
};

class Sink :
        public Gst::BaseSink
{
protected:
        explicit Sink(GstBaseSink *gobj);
};

class Device :
        public Gst::Element
{
        /* Operations specific to hardware devices go here */
protected:
        explicit Device(GstElement *gobj);
};

class CaptureDevice :
        public Device
{
        /* Operations specific to capture (source) devices go here */
protected:
        explicit CaptureDevice(GstElement *gobj);
};

#ifdef HAVE_LIBSIGROKCXX
class LegacyCaptureDevice :
        public CaptureDevice
{
public:
        /* Create from libsigrok device object */
        static Glib::RefPtr<LegacyCaptureDevice> create(
                shared_ptr<sigrok::HardwareDevice> libsigrok_device);

        /* Retrieve libsigrok device object */
        shared_ptr<sigrok::HardwareDevice> libsigrok_device();

        /* Override state change */
        Gst::StateChangeReturn change_state_vfunc(Gst::StateChange transition);

        /* Gst class init */
        static void class_init(Gst::ElementClass<LegacyCaptureDevice> *klass);

        /* Register class with plugin */
        static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

        /* Construcor used by element factory */
        explicit LegacyCaptureDevice(GstElement *gobj);
private:
        shared_ptr<sigrok::HardwareDevice> _libsigrok_device;
        Glib::RefPtr<Gst::Pad> _src_pad;
        Glib::Threads::RecMutex _mutex;
        Glib::RefPtr<Gst::Task> _task;
        shared_ptr<sigrok::Session> _session;

        void _datafeed_callback(shared_ptr<sigrok::Device> device,
                        shared_ptr<sigrok::Packet> packet);
        void _run();
};

class LegacyInput :
        public Gst::Element
{
public:
        /* Create from libsigrok input */
        static Glib::RefPtr<LegacyInput> create(
                shared_ptr<sigrok::InputFormat> format,
                map<string, Glib::VariantBase> options = map<string, Glib::VariantBase>());

        /* Override start */
        bool start_vfunc();

        /* Chain function */
        Gst::FlowReturn chain(const Glib::RefPtr<Gst::Pad> &pad,
                        const Glib::RefPtr<Gst::Buffer> &buf);

        /* Override stop */
        bool stop_vfunc();

        /* Gst class init */
        static void class_init(Gst::ElementClass<LegacyInput> *klass);

        /* Register class with plugin */
        static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

        /* Construcor used by element factory */
        explicit LegacyInput(GstElement *gobj);
private:
        shared_ptr<sigrok::InputFormat> _libsigrok_input_format;
        shared_ptr<sigrok::Input> _libsigrok_input;
        shared_ptr<sigrok::Session> _session;
        map<string, Glib::VariantBase> _options;
        Glib::RefPtr<Gst::Pad> _sink_pad;
        Glib::RefPtr<Gst::Pad> _src_pad;

        void _datafeed_callback(shared_ptr<sigrok::Device> device,
                        shared_ptr<sigrok::Packet> packet);
};

class LegacyOutput :
        public Sink
{
public:
        /* Create from libsigrok output object */
        static Glib::RefPtr<LegacyOutput> create(
                shared_ptr<sigrok::OutputFormat> libsigrok_output_format,
                shared_ptr<sigrok::Device> libsigrok_device,
                map<string, Glib::VariantBase> options = map<string, Glib::VariantBase>());

        /* Override start */
        bool start_vfunc();

        /* Override render */
        Gst::FlowReturn render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer);

        /* Override stop */
        bool stop_vfunc();

        /* Gst class init */
        static void class_init(Gst::ElementClass<LegacyOutput> *klass);

        /* Register class with plugin */
        static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

        /* Constructor used by element factory */
        explicit LegacyOutput(GstBaseSink *gobj);
private:
        shared_ptr<sigrok::OutputFormat> _libsigrok_output_format;
        shared_ptr<sigrok::Device> _libsigrok_device;
        shared_ptr<sigrok::Output> _libsigrok_output;
        map<string, Glib::VariantBase> _options;
};
#endif

#ifdef HAVE_LIBSIGROKDECODE
class LegacyDecoder :
        public Sink
{
public:
        static Glib::RefPtr<LegacyDecoder> create(
                struct srd_session *libsigrokdecode_session, uint64_t unitsize);

        /* Retrieve libsigrokdecode session */
        struct srd_session *libsigrokdecode_session();

        /* Override start */
        bool start_vfunc();

        /* Override render */
        Gst::FlowReturn render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer);

        /* Override stop */
        bool stop_vfunc();

        /* Gst class init */
        static void class_init(Gst::ElementClass<LegacyDecoder> *klass);

        /* Register class with plugin */
        static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

        /* Constructor used by element factory */
        explicit LegacyDecoder(GstBaseSink *gobj);
private:
        struct srd_session *_session;
        uint64_t _abs_ss;
        uint64_t _unitsize;
};
#endif

}
#endif
