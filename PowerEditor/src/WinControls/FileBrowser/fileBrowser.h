// This file is part of Notepad++ project
// Copyright (C)2003 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid      
// misunderstandings, we consider an application to constitute a          
// "derivative work" for the purpose of this license if it does any of the
// following:                                                             
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#pragma once

#include "DockingDlgInterface.h"
#include "TreeView.h"
#include "fileBrowser_rc.h"

#define FB_PANELTITLE         L"Folder as Workspace"
#define FB_ADDROOT            L"Add"
#define FB_REMOVEALLROOTS     L"Remove All"
#define FB_REMOVEROOTFOLDER   L"Remove"
#define FB_COPYPATH           L"Copy path"
#define FB_COPYFILENAME       L"Copy file name"
#define FB_FINDINFILES        L"Find in Files..."
#define FB_EXPLORERHERE       L"Explorer here"
#define FB_CMDHERE            L"CMD here"
#define FB_OPENINNPP          L"Open"
#define FB_SHELLEXECUTE       L"Run by system"

class TiXmlNode;
class FileBrowser;
class FolderInfo;

class FileInfo final	{

friend class FileBrowser;
friend class FolderInfo;

public:
	FileInfo() = delete; // constructor by default is forbidden
	FileInfo(const generic_string & name, FolderInfo *parent) : _name(name), _parent(parent) {};
	generic_string getName() const { return _name; };
	void setName(generic_string name) { _name = name; };

private:
	FolderInfo *_parent = nullptr;
	generic_string _name;
};


class FolderInfo final	{

friend class FileBrowser;
friend class FolderUpdater;

public:
	FolderInfo() = delete; // constructor by default is forbidden
	FolderInfo(const generic_string & name, FolderInfo *parent) : _name(name), _parent(parent) {};
	void setRootPath(const generic_string& rootPath) { _rootPath = rootPath; };
	generic_string getRootPath() const { return _rootPath; };
	void setName(const generic_string& name) { _name = name; };
	generic_string getName() const { return _name; };
	void addFile(const generic_string& fn) { _files.push_back(FileInfo(fn, this)); };
	void addSubFolder(FolderInfo subDirectoryStructure) { _subFolders.push_back(subDirectoryStructure); };

	bool addToStructure(generic_string & fullpath, std::vector<generic_string> linarPathArray);
	bool removeFromStructure(std::vector<generic_string> linarPathArray);
	bool renameInStructure(std::vector<generic_string> linarPathArrayFrom, std::vector<generic_string> linarPathArrayTo);

private:
	std::vector<FolderInfo> _subFolders;
	std::vector<FileInfo> _files;
	FolderInfo *_parent = nullptr;
	generic_string _name;
	generic_string _rootPath; // set only for root folder; empty for normal folder
};

enum BrowserNodeType {
	browserNodeType_root = 0, browserNodeType_folder = 2, browserNodeType_file = 3
};

class FolderUpdater {
friend class FileBrowser;
public:
	FolderUpdater(const FolderInfo& fi, FileBrowser *pFileBrowser) : _rootFolder(fi), _pFileBrowser(pFileBrowser) {};
	~FolderUpdater() = default;

	void startWatcher();
	void stopWatcher();

private:
	FolderInfo _rootFolder;
	FileBrowser *_pFileBrowser = nullptr;
	HANDLE _watchThreadHandle = nullptr;
	HANDLE _EventHandle = nullptr;
	static DWORD WINAPI watching(void *param);
};

class FileBrowser : public DockingDlgInterface {
public:
	FileBrowser(): DockingDlgInterface(IDD_FILEBROWSER) {};
	~FileBrowser();
	void init(HINSTANCE hInst, HWND hPere) {
		DockingDlgInterface::init(hInst, hPere);
	}

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
    };

    void setParent(HWND parent2set){
        _hParent = parent2set;
    };

	virtual void setBackgroundColor(COLORREF bgColour) {
		TreeView_SetBkColor(_treeView.getHSelf(), bgColour);
    };

	virtual void setForegroundColor(COLORREF fgColour) {
		TreeView_SetTextColor(_treeView.getHSelf(), fgColour);
    };

	generic_string getNodePath(HTREEITEM node) const;
	generic_string getNodeName(HTREEITEM node) const;
	void addRootFolder(generic_string);

	HTREEITEM getRootFromFullPath(const generic_string & rootPath) const;
	HTREEITEM findChildNodeFromName(HTREEITEM parent, const generic_string&) const;

	bool addInTree(const generic_string& rootPath, const generic_string& addItemFullPath, HTREEITEM node, std::vector<generic_string> linarPathArray);
	HTREEITEM findInTree(const generic_string& rootPath, HTREEITEM node, std::vector<generic_string> linarPathArray) const;
	bool deleteFromTree(const generic_string& rootPath, HTREEITEM node, const std::vector<generic_string>& linarPathArray);
	void deleteAllFromTree() {
		popupMenuCmd(IDM_FILEBROWSER_REMOVEALLROOTS);
	};

	bool renameInTree(const generic_string& rootPath, HTREEITEM node, const std::vector<generic_string>& linarPathArrayFrom, const generic_string & renameTo);

	std::vector<generic_string> getRoots() const;
	generic_string getSelectedItemPath() const;

protected:
	HWND _hToolbarMenu = nullptr;

	TreeView _treeView;
	HIMAGELIST _hImaLst = nullptr;

	HMENU _hGlobalMenu = NULL;
	HMENU _hRootMenu = NULL;
	HMENU _hFolderMenu = NULL;
	HMENU _hFileMenu = NULL;
	std::vector<FolderUpdater *> _folderUpdaters;

	void initPopupMenus();
	void destroyMenus();
	BOOL setImageList(int root_open_id, int root_close_id, int open_node_id, int closed_node_id, int leaf_id);

	BrowserNodeType getNodeType(HTREEITEM hItem);
	void popupMenuCmd(int cmdID);
	bool selectCurrentEditingFile() const;

	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	void notified(LPNMHDR notification);
	void showContextMenu(int x, int y);
	void openSelectFile();
	void getDirectoryStructure(const TCHAR *dir, const std::vector<generic_string> & patterns, FolderInfo & directoryStructure, bool isRecursive, bool isInHiddenDir); 
	HTREEITEM createFolderItemsFromDirStruct(HTREEITEM hParentItem, const FolderInfo & directoryStructure);
};
