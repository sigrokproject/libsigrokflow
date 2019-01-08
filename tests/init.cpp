/*
 * This file is part of the libsigrokflow project.
 *
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

#include <catch2/catch.hpp>
#include <libsigrokflow/libsigrokflow.hpp>

TEST_CASE("init/deinit", "[init]")
{
	REQUIRE_NOTHROW(Gst::init());

	SECTION("Srf::init() after successful Gst::init() should not throw") {
		CHECK_NOTHROW(Srf::init());
		SECTION("Multiple Srf::init() calls should throw") {
			for (int i = 0; i < 10; ++i)
				CHECK_THROWS(Srf::init());
			CHECK_NOTHROW(Srf::deinit());
		}
		SECTION("Srf::deinit() should not throw") {
			CHECK_NOTHROW(Srf::deinit());
		}
		SECTION("Multiple Srf::deinit() calls should throw") {
			CHECK_NOTHROW(Srf::deinit());
			for (int i = 0; i < 10; ++i)
				CHECK_THROWS(Srf::deinit());
		}
	}

	REQUIRE_NOTHROW(Gst::deinit());
}
