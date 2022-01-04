#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/pixmap/Pixmap.hh>
#include <imagine/util/DelegateFunc.hh>

namespace IG::Data
{

class PixmapSource
{
public:
	using WriteDelegate = DelegateFunc<void(IG::Pixmap dest)>;

	constexpr PixmapSource() = default;
	constexpr PixmapSource(IG::Pixmap pix):pix{pix} {}
	constexpr PixmapSource(WriteDelegate del, IG::Pixmap pix):
		writeDel{del}, pix{pix} {}
	constexpr void write(IG::Pixmap dest) { writeDel(dest); }
	constexpr IG::Pixmap pixmapView() { return pix; }

protected:
	WriteDelegate writeDel{};
	IG::Pixmap pix{};
};

}