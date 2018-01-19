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

#include "io.h"

bool fileExist(FS_Archive archive, std::u16string path)
{
	FSStream stream(archive, path, FS_OPEN_READ);
	bool exist = stream.getLoaded();
	stream.close();
	return exist;
}

void copyFile(FS_Archive srcArch, FS_Archive dstArch, std::u16string srcPath, std::u16string dstPath)
{
	u32 size = 0;
	FSStream input(srcArch, srcPath, FS_OPEN_READ);
	if (input.getLoaded())
	{
		size = input.getSize() > BUFFER_SIZE ? BUFFER_SIZE : input.getSize();
	}
	else
	{
		return;
	}
	
	FSStream output(dstArch, dstPath, FS_OPEN_WRITE, input.getSize());
	if (output.getLoaded())
	{
		u8* buf = new u8[size];
		do {
			u32 rd = input.read(buf, size);
			output.write(buf, rd);
			size_t slashpos = srcPath.rfind(u8tou16("/"));
			drawCopy(srcPath.substr(slashpos + 1, srcPath.length() - slashpos - 1), input.getOffset(), input.getSize());
		} while(!input.isEndOfFile());
		delete[] buf;		
	}

	input.close();
	output.close();
}

Result copyDirectory(FS_Archive srcArch, FS_Archive dstArch, std::u16string srcPath, std::u16string dstPath)
{
	Result res = 0;
	bool quit = false;
	Directory items(srcArch, srcPath);
	
	if (!items.getLoaded())
	{
		return items.getError();
	}
	
	for (size_t i = 0, sz = items.getCount(); i < sz && !quit; i++)
	{
		std::u16string newsrc = srcPath + items.getItem(i);
		std::u16string newdst = dstPath + items.getItem(i);
		
		if (items.isFolder(i))
		{
			res = createDirectory(dstArch, newdst);
			if (R_SUCCEEDED(res) || (u32)res == 0xC82044B9)
			{
				newsrc += u8tou16("/");
				newdst += u8tou16("/");
				res = copyDirectory(srcArch, dstArch, newsrc, newdst);
			}
			else
			{
				quit = true;
			}
		}
		else
		{
			copyFile(srcArch, dstArch, newsrc, newdst);
		}
	}
	
	return res;
}

Result createDirectory(FS_Archive archive, std::u16string path)
{
	return FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path.data()), 0);
}

bool directoryExist(FS_Archive archive, std::u16string path)
{
	Handle handle;

	if (R_FAILED(FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_UTF16, path.data()))))
	{
		return false;
	}

	if (R_FAILED(FSDIR_Close(handle)))
	{
		return false;
	}

	return true;
}

void copyPatchFolder(size_t index)
{
	if (getCellsCount() == 0)
	{
		return;
	}
	
	Title title;
	getTitle(title, index);
	const size_t cellIndex = getScrollableIndex();
	
	std::u16string srcPath = title.folderPath + u8tou16("/") + u8tou16(getPathFromCell(cellIndex).c_str()) + u8tou16("/");
	std::u16string dstPath = title.patchPath + u8tou16("/");

	const FS_Archive sdmc = getArchiveSDMC();
	if (fileExist(sdmc, title.folderPath + u8tou16("/lastloaded.json")))
	{
		std::ifstream i(u16tou8(title.folderPath) + "/lastloaded.json");
		nlohmann::json j;
		i >> j;
		
		for (auto& obj : j)
		{
			std::u16string name = u8tou16(obj["name"].get<std::string>().c_str());
			bool folder = obj["folder"];
			std::u16string delpath = dstPath + name;
			
			Result res;
			if (folder)
			{
				res = FSUSER_DeleteDirectoryRecursively(sdmc, fsMakePath(PATH_UTF16, delpath.data()));
			}
			else
			{
				res = FSUSER_DeleteFile(sdmc, fsMakePath(PATH_UTF16, delpath.data()));
			}
			
			if (R_FAILED(res))
			{
				createError(res, "Failed to delete previously loaded patch.");
				return;
			}
		}
	}
	
	Result res = copyDirectory(sdmc, sdmc, srcPath, dstPath);
	if (R_FAILED(res))
	{
		std::string message = "Failed to copy patch directory.";
		createError(res, message);
		
		FSUSER_DeleteDirectoryRecursively(sdmc, fsMakePath(PATH_UTF16, dstPath.data()));
		return;
	}
	
	// save last loaded patch path to disk
	Directory dir(sdmc, srcPath);
	nlohmann::json j = nlohmann::json::array();
	for (size_t i = 0; i < dir.getCount(); i++)
	{
		nlohmann::json t;
		t["name"] = u16tou8(dir.getItem(i));
		t["folder"] = dir.isFolder(i);
		j.push_back(t);
	}
	std::ofstream o(u16tou8(title.folderPath) + "/lastloaded.json");
	o << j;
	
	createInfo("Success!", "Patch correctly copied to Luma3DS's path.");
}

void deletePatchFolder(size_t index)
{
	if (getCellsCount() == 0)
	{
		return;
	}
	
	Title title;
	getTitle(title, index);
	std::u16string dstPath = title.patchPath + u8tou16("/");
	
	const FS_Archive sdmc = getArchiveSDMC();
	if (fileExist(sdmc, title.folderPath + u8tou16("/lastloaded.json")))
	{
		std::ifstream i(u16tou8(title.folderPath) + "/lastloaded.json");
		nlohmann::json j;
		i >> j;
		
		for (auto& obj : j)
		{
			std::u16string name = u8tou16(obj["name"].get<std::string>().c_str());
			bool folder = obj["folder"];
			std::u16string delpath = dstPath + name;
			
			Result res;
			if (folder)
			{
				res = FSUSER_DeleteDirectoryRecursively(sdmc, fsMakePath(PATH_UTF16, delpath.data()));
			}
			else
			{
				res = FSUSER_DeleteFile(sdmc, fsMakePath(PATH_UTF16, delpath.data()));
			}
			
			if (R_FAILED(res))
			{
				createError(res, "Failed to delete previously loaded patch.");
				return;
			}
		}
	}
	
	FSUSER_DeleteFile(sdmc, fsMakePath(PATH_UTF16, (title.folderPath + u8tou16("/lastloaded.json")).data()));
	
	createInfo("Success!", "Old patch correctly deleted from Luma3DS's path.");
}