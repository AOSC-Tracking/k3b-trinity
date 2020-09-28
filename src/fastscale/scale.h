/*
Copied from gwenview/src/imageutils/imageutils.h
Copyright 2000-2004 Aurélien Gâteau
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#ifndef FAST_SCALE_H
#define FAST_SCALE_H

// TQt
#include <tqimage.h>

namespace ImageUtils {
	enum SmoothAlgorithm { SMOOTH_NONE, SMOOTH_FAST, SMOOTH_NORMAL, SMOOTH_BEST };

	TQImage scale(const TQImage& image, int width, int height,
		SmoothAlgorithm alg, TQ_ScaleMode mode = TQ_ScaleFree, double blur = 1.0);
}

#endif
