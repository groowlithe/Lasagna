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

#include "gui.h"

static Info info;
static Clickable* buttonClear;
static Clickable* buttonLoad;
static MessageBox* messageBox;
static MessageBox* copyList;
static Scrollable* directoryList;

static void drawBasicInterface(gfxScreen_t screen);

/// Gui implementation

static void drawBasicInterface(gfxScreen_t screen)
{
	if (screen == GFX_TOP)
	{
		pp2d_draw_rectangle(0, 0, 400, 19, COLOR_BARS);
		pp2d_draw_rectangle(0, 221, 400, 19, COLOR_BARS);
		
		char version[10];
		sprintf(version, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
		static float versionLen = pp2d_get_text_width(version, 0.45f, 0.45f);
		static float checkpointLen = pp2d_get_text_width("Lasagna", 0.50f, 0.50f);

		pp2d_draw_text(4, 3, 0.45f, 0.45f, COLOR_LIGHT, getTime().c_str());
		pp2d_draw_text(TOP_WIDTH - 4 - versionLen, 3, 0.45f, 0.45f, COLOR_LIGHT, version);
		pp2d_draw_texture(TEXTURE_ICON, TOP_WIDTH - 5 - versionLen - 19, 0);
		pp2d_draw_text(TOP_WIDTH - 6 - versionLen - checkpointLen - 19, 2, 0.50f, 0.50f, COLOR_LIGHT, "Lasagna");		
	}
	else
	{
		pp2d_draw_rectangle(0, 0, 320, 19, COLOR_BARS);
		pp2d_draw_rectangle(0, 221, 320, 19, COLOR_BARS);		
	}
}

void drawCopy(std::u16string src, u32 offset, u32 size)
{
	copyList->clear();
	copyList->push_message("Copying " + u16tou8(src));
	
	pp2d_begin_draw(GFX_TOP, GFX_LEFT);
		drawBasicInterface(GFX_TOP);
		copyList->draw();
		
		pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
		drawBasicInterface(GFX_BOTTOM);
		
		static const int barHeight = 19;
		static const int progressBarHeight = 50;
		static const int spacingFromSides = 20;
		static const int spacingFromBars = (240 - barHeight * 2 - progressBarHeight) / 2;
		static const int width = 320 - spacingFromSides * 2;
		
		pp2d_draw_rectangle(spacingFromSides - 2, barHeight + spacingFromBars - 2, width + 4, progressBarHeight + 4, GREYISH);
		pp2d_draw_rectangle(spacingFromSides, barHeight + spacingFromBars, width, progressBarHeight, WHITE);
		pp2d_draw_rectangle(spacingFromSides, barHeight + spacingFromBars, (float)offset / (float)size * width, progressBarHeight, RGBA8(116, 222, 126, 255));
		
		std::string sizeString = getSizeString(offset) + " of " + getSizeString(size);
		pp2d_draw_text_center(GFX_BOTTOM, 112, 0.5f, 0.5f, BLACK, sizeString.c_str());
	pp2d_end_draw();
}

bool askForConfirmation(std::string text)
{
	Clickable buttonYes(40, 90, 100, 60, COLOR_LIGHT, BLACK, "   \uE000 Yes", true);
	Clickable buttonNo(180, 90, 100, 60, COLOR_LIGHT, BLACK, "   \uE001 No", true);
	MessageBox message(COLOR_BARS, COLOR_LIGHT, GFX_TOP);
	message.push_message(text);
	
	while(aptMainLoop() && !(buttonNo.isReleased() || hidKeysDown() & KEY_B))
	{
		hidScanInput();
		if (buttonYes.isReleased() || hidKeysDown() & KEY_A)
		{
			return true;
		}
		
		pp2d_begin_draw(GFX_TOP, GFX_LEFT);
			drawBasicInterface(GFX_TOP);
			message.draw();
			pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
			drawBasicInterface(GFX_BOTTOM);
			pp2d_draw_rectangle(38, 88, 104, 64, GREYISH);
			pp2d_draw_rectangle(178, 88, 104, 64, GREYISH);
			buttonYes.draw();
			buttonNo.draw();
		pp2d_end_draw();
	}
	
	return false;
}

void createInfo(std::string title, std::string message)
{
	info.init(title, message, 500, TYPE_INFO);
}

void createError(Result res, std::string message)
{
	info.init(res, message, 500, TYPE_ERROR);
}

void resetDirectoryListIndex(void)
{
	directoryList->resetIndex();
}

size_t getScrollableIndex(void)
{
	return directoryList->getIndex();
}

void setScrollableIndex(size_t index)
{
	directoryList->setIndex(index);
}

Gui::Gui(void)
{
	index = 0;
	page = 0;
	bottomScrollEnabled = false;
	info.init("", "", 0, TYPE_INFO);
	buttonClear = new Clickable(204, 102, 110, 54, COLOR_LIGHT, bottomScrollEnabled ? BLUE : GREYISH, "Remove \uE071", true);
	buttonLoad = new Clickable(204, 158, 110, 54, COLOR_LIGHT, bottomScrollEnabled ? BLUE : GREYISH, "  Load \uE008", true);
	messageBox = new MessageBox(BLUE, COLOR_LIGHT, GFX_TOP);
	copyList = new MessageBox(BLUE, COLOR_LIGHT, GFX_TOP);
	directoryList = new Scrollable(6, 102, 196, 110, 5);
	
	messageBox->push_message("Press \uE000 to enter target.");
	messageBox->push_message("Press \uE001 to exit target.");
	messageBox->push_message("Press \uE006 to move between titles.");
	messageBox->push_message("Press \uE004\uE005 to switch page.");
}

bool Gui::getBottomScroll(void)
{
	return bottomScrollEnabled;
}

size_t Gui::getFullIndex(void)
{
	return index + page*rowlen*collen;
}

void Gui::setBottomScroll(bool enable)
{
	bottomScrollEnabled = enable;
}

void Gui::updateButtonsColor(void)
{
	if (bottomScrollEnabled)
	{
		buttonClear->setColors(COLOR_LIGHT, BLACK);
		buttonLoad->setColors(COLOR_LIGHT, BLACK);
	}
	else
	{
		buttonClear->setColors(COLOR_LIGHT, GREYISH);
		buttonLoad->setColors(COLOR_LIGHT, GREYISH);		
	}
}

void Gui::updateSelector(void)
{
	if (!getBottomScroll())
	{
		const size_t entries = rowlen * collen;
		const size_t maxentries = (getTitlesCount() - page*entries) > entries ? entries : getTitlesCount() - page*entries;
		const size_t maxpages = getTitlesCount() / entries + 1;
		calculateIndex(index, page, maxpages, maxentries, entries, collen);

		directoryList->resetIndex();
	}
	else
	{
		directoryList->updateSelection();
	}
}

void Gui::drawSelector(void)
{
	static const int w = 2;
	const int x = getSelectorX(index);
	const int y = getSelectorY(index);
	pp2d_draw_rectangle(         x,          y, 50,       50, RGBA8(255, 255, 255, 200)); 
	pp2d_draw_rectangle(         x,          y, 50,        w, BLUE); // top
	pp2d_draw_rectangle(         x,      y + w,  w, 50 - 2*w, BLUE); // left
	pp2d_draw_rectangle(x + 50 - w,      y + w,  w, 50 - 2*w, BLUE); // right
	pp2d_draw_rectangle(         x, y + 50 - w, 50,        w, BLUE); // bottom
}

int Gui::getSelectorX(size_t index)
{
	return 50*((index % (rowlen*collen)) % collen);
}

int Gui::getSelectorY(size_t index)
{
	return 20 + 50*((index % (rowlen*collen)) / collen);
}

void Gui::draw(void)
{
	const size_t entries = rowlen * collen;
	const size_t max = (getTitlesCount() - page*entries) > entries ? entries : getTitlesCount() - page*entries;
	
	pp2d_begin_draw(GFX_TOP, GFX_LEFT);
		drawBasicInterface(GFX_TOP);
		
		for (size_t k = page*entries; k < page*entries + max; k++)
		{
			pp2d_draw_texture(getTextureId(k), getSelectorX(k) + 1, getSelectorY(k) + 1);
		}
		
		if (getTitlesCount() > 0)
		{
			drawSelector();
		}
		
		pp2d_draw_text_center(GFX_TOP, 224, 0.47f, 0.47f, COLOR_LIGHT, "Hold SELECT to see commands.");
		
		info.draw();
		
		if (hidKeysHeld() & KEY_SELECT)
		{
			messageBox->draw();
		}
		
		pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
		drawBasicInterface(GFX_BOTTOM);
		
		if (getTitlesCount() > 0)
		{
			Title title;
			getTitle(title, index + page*entries);
			
			directoryList->flush();
			std::vector<std::u16string> dirs = title.directories;
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
			
			for (size_t i = 0; i < dirs.size(); i++)
			{
				directoryList->addCell(COLOR_LIGHT, bottomScrollEnabled ? BLUE : GREYISH, convert.to_bytes(dirs.at(i)));
				if (i == directoryList->getIndex())
				{
					directoryList->invertCellColors(i);
				}
			}
			
			pp2d_draw_text(4, 1, 0.6f, 0.6f, COLOR_LIGHT, title.getShortDescription().c_str());
			pp2d_draw_text_wrap(4, 27, 0.55f, 0.55f, WHITE, 240, title.getLongDescription().c_str());
			
			float longDescrHeight = pp2d_get_text_height_wrap(title.getLongDescription().c_str(), 0.55f, 0.55f, 240);
			pp2d_draw_text(4, 31 + longDescrHeight, 0.5f, 0.5f, WHITE, "ID:");
			
			char lowid[9];
			snprintf(lowid, 9, "%08X", (int)title.getLowId());
			pp2d_draw_text(25, 31 + longDescrHeight, 0.5f, 0.5f, COLOR_LIGHT, lowid);
			pp2d_draw_textf(30 + pp2d_get_text_width(lowid, 0.5f, 0.5f), 32 + longDescrHeight, 0.42f, 0.42f, WHITE, "(%s)", title.productCode);
			pp2d_draw_text(4, 47 + longDescrHeight, 0.5f, 0.5f, WHITE, "Mediatype:");
			pp2d_draw_textf(75, 47 + longDescrHeight, 0.5f, 0.5f, COLOR_LIGHT, "%s", title.getMediatypeString().c_str());
			
			pp2d_draw_rectangle(260, 27, 52, 52, BLACK);
			pp2d_draw_texture(title.textureId, 262, 29);
			
			pp2d_draw_rectangle(4, 100, 312, 114, BLUE);
			pp2d_draw_rectangle(6, 102, 308, 110, COLOR_BARS);

			directoryList->draw();
			buttonClear->draw();
			buttonLoad->draw();
			
			pp2d_draw_rectangle(202, 102, 2, 110, BLUE);
			pp2d_draw_rectangle(204, 156, 110, 2, BLUE);
		}
		
		pp2d_draw_text_center(GFX_BOTTOM, 224, 0.46f, 0.46f, COLOR_LIGHT, "Press \uE073 or START to exit.");
	pp2d_end_draw();
}

bool Gui::isClearReleased(void)
{
	return buttonClear->isReleased() && bottomScrollEnabled;
}

bool Gui::isLoadReleased(void)
{
	return buttonLoad->isReleased() && bottomScrollEnabled;
}

void Gui::resetIndex(void)
{
	index = 0;
	page = 0;
}