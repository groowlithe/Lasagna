/*  This file is part of Lasagna
>	Copyright (C) 2018 Bernardo Giordano
>
>   This program is free software: you can redistribute it and/or modify
>   it under the terms of the GNU General Public License as published by
>   the Free Software Foundation, either version 3 of the License, or
>   (at your option) any later version.
>
>   This program is distributed in the hope that it will be useful,
>   but WITHOUT ANY WARRANTY; without even the implied warranty of
>   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>   GNU General Public License for more details.
>
>   You should have received a copy of the GNU General Public License
>   along with this program.  If not, see <http://www.gnu.org/licenses/>.
>   See LICENSE for information.
*/

#ifndef GUI_H
#define GUI_H

#include "common.h"

#define TEXTURE_ICON 1
#define TEXTURE_NOICON 2
#define TEXTURE_FIRST_FREE_INDEX 3

#define COLOR_BACKGROUND ABGR8(255, 0x37, 0x32, 0x23)
#define COLOR_BARS RGBA8(0x98, 0x4B, 0x43, 255)
#define COLOR_LIGHT RGBA8(0xEA, 0xC6, 0x7A, 255)
#define GREYISH RGBA8(157, 157, 157, 255)
#define BLUE RGBA8(0x18, 0x12, 0x1E, 255)
#define BLACK RGBA8(0, 0, 0, 255)
#define RED RGBA8(255, 0, 0, 255)
#define GREEN RGBA8(0, 255, 0, 255)
#define WHITE RGBA8(255, 255, 255, 255)

bool askForConfirmation(std::string text);
void drawCopy(std::u16string src, u32 offset, u32 size);
void createInfo(std::string title, std::string message);
void createError(Result res, std::string message);

size_t getScrollableIndex(void);
void resetDirectoryListIndex(void);
void setScrollableIndex(size_t index);

class Gui
{
public:
	Gui(void);
	bool getBottomScroll(void);
	size_t getFullIndex(void);
	bool isClearReleased(void);
	bool isLoadReleased(void);
	void setBottomScroll(bool enable);
	void updateButtonsColor(void);
	void updateSelector(void);
	void resetIndex(void);
	
	void draw(void);

private:
	void drawSelector(void);
	int getSelectorX(size_t index);
	int getSelectorY(size_t index);
	
	size_t index;
	int page;
	const size_t rowlen = 4;
	const size_t collen = 8;
	
	bool bottomScrollEnabled;
};

#endif