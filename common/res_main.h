// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: res_main.h $
//
// Copyright (C) 2006-2014 by The Odamex Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//
// Game resource file management, including WAD files.
//
//-----------------------------------------------------------------------------

#ifndef __RES_MAIN_H__
#define __RES_MAIN_H__

#include <stdlib.h>
#include "m_ostring.h"
#include "hashtable.h"
#include <vector>
#include <string>

#include "res_resourcepath.h"

#include "w_wad.h"

// Typedefs
typedef uint32_t ResourceId;
typedef uint32_t ResourceFileId;
typedef uint32_t ResourceContainerId;
typedef uint32_t LumpId;


// Forward class declarations
class ResourceManager;
class ResourceContainer;
class FileAccessor;
class ContainerDirectory;


// Default directory names for ZDoom zipped resource files.
// See: http://zdoom.org/wiki/Using_ZIPs_as_WAD_replacement
//
static const ResourcePath global_directory_name("/GLOBAL/");
static const ResourcePath patches_directory_name("/PATCHES/");
static const ResourcePath graphics_directory_name("/GRAPHICS/");
static const ResourcePath sounds_directory_name("/SOUNDS/");
static const ResourcePath music_directory_name("/MUSIC/");
static const ResourcePath maps_directory_name("/MAPS/");
static const ResourcePath flats_directory_name("/FLATS/");
static const ResourcePath sprites_directory_name("/SPRITES/");
static const ResourcePath textures_directory_name("/TEXTURES/");
static const ResourcePath hires_directory_name("/HIRES/");
static const ResourcePath colormaps_directory_name("/COLORMAPS/");
static const ResourcePath acs_directory_name("/ACS/");
static const ResourcePath voices_directory_name("/VOICES/");
static const ResourcePath voxels_directory_name("/VOXELS/");



typedef std::vector<ResourceId> ResourceIdList;



// ============================================================================
//
// ResourceContainer abstract base class interface
//
// ============================================================================

class ResourceContainer
{
public:
	ResourceContainer() { }
	virtual ~ResourceContainer() { }

	virtual const ResourceContainerId& getResourceContainerId() const = 0;

	virtual bool isIWad() const { return false; }

	virtual size_t getLumpCount() const = 0;

	virtual size_t getLumpLength(const LumpId lump_id) const = 0;

	virtual size_t readLump(const LumpId lump_id, void* data, size_t length) const = 0;
};

// ============================================================================
//
// SingleLumpResourceContainer abstract base class interface
//
// ============================================================================

class SingleLumpResourceContainer : public ResourceContainer
{
public:
	SingleLumpResourceContainer(
			FileAccessor* file,
			const ResourceContainerId& container_id,
			ResourceManager* manager);

	virtual ~SingleLumpResourceContainer() {}

	virtual const ResourceContainerId& getResourceContainerId() const
	{
		return mResourceContainerId;
	}

	virtual size_t getLumpCount() const;

	virtual size_t getLumpLength(const LumpId lump_id) const;

	virtual size_t readLump(const LumpId lump_id, void* data, size_t length) const;

private:
	ResourceContainerId		mResourceContainerId;
	FileAccessor*			mFile;
};


// ============================================================================
//
// WadResourceContainer abstract base class interface
//
// ============================================================================

class WadResourceContainer : public ResourceContainer
{
public:
	WadResourceContainer(
			FileAccessor* file,
			const ResourceContainerId& container_id,
			ResourceManager* manager);
	
	virtual ~WadResourceContainer();

	virtual const ResourceContainerId& getResourceContainerId() const
	{
		return mResourceContainerId;
	}

	virtual bool isIWad() const
	{
		return mIsIWad;
	}

	virtual size_t getLumpCount() const;

	virtual size_t getLumpLength(const LumpId lump_id) const;
		
	virtual size_t readLump(const LumpId lump_id, void* data, size_t length) const;

private:
	void cleanup();

	ResourceContainerId		mResourceContainerId;
	FileAccessor*			mFile;

	ContainerDirectory*		mDirectory;

	bool					mIsIWad;
};


// ============================================================================
//
// ResourceManager class interface
//
// ============================================================================
//
// Manages a collection of ResourceFiles.
//

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	static const ResourceId RESOURCE_NOT_FOUND = 0;

	const std::vector<std::string>& getResourceFileNames() const
	{
		return mResourceFileNames;
	}

	const std::vector<std::string>& getResourceFileHashes() const
	{
		return mResourceFileHashes;
	}

	void openResourceFiles(const std::vector<std::string>& filenames);

	void closeAllResourceFiles();

	const ResourceId addResource(
			const ResourcePath& path,
			const ResourceContainer* container,
			const LumpId lump_id);

	bool validateResourceId(const ResourceId res_id) const
	{
		return mResources.validate(res_id);
	}

	const ResourceId getResourceId(const ResourcePath& path) const;
	const ResourceId getResourceId(const OString& name, const OString& directory) const
	{
		return getResourceId(Res_MakeResourcePath(name, directory));
	}

	const ResourceIdList getAllResourceIds(const ResourcePath& path) const;
	const ResourceIdList getAllResourceIds(const OString& name, const OString& directory) const
	{
		return getAllResourceIds(Res_MakeResourcePath(name, directory));
	}

	const ResourcePath& getResourcePath(const ResourceId res_id) const
	{
		const ResourceRecord* res_rec = getResourceRecord(res_id);
		if (res_rec)
			return res_rec->mPath;

		static const ResourcePath empty_path;
		return empty_path;
	}

	size_t getLumpLength(const ResourceId res_id) const;

	size_t readLump(const ResourceId res_id, void* data) const;


	const ResourceContainer* getResourceContainer(const ResourceContainerId& container_id) const
	{
		if (container_id < mContainers.size())
			return mContainers[container_id];
		return NULL;
	}

	const std::string& getResourceContainerFileName(const ResourceId res_id) const;

	void dump() const;


private:
	struct ResourceRecord
	{
		ResourceRecord& operator=(const ResourceRecord& other)
		{
			if (&other != this)
			{
				mPath = other.mPath;
				mResourceContainerId = other.mResourceContainerId;
				mLumpId = other.mLumpId;
			}
			return *this;
		}

		ResourcePath		mPath;
		ResourceContainerId	mResourceContainerId;
		LumpId				mLumpId;
	};

	typedef SArray<ResourceRecord> ResourceRecordTable;
	ResourceRecordTable		mResources;

	// ---------------------------------------------------------------------------
	// Private helper functions
	// ---------------------------------------------------------------------------

	const ResourceRecord* getResourceRecord(const ResourceId res_id) const
	{
		if (mResources.validate(res_id))
			return &mResources.get(res_id);
		return NULL;
	}

	const ResourceContainerId& getResourceContainerId(const ResourceId res_id) const
	{
		const ResourceRecord* res_rec = getResourceRecord(res_id);
		if (res_rec)
			return res_rec->mResourceContainerId;

		static const ResourceContainerId invalid_container_id = -1;
		return invalid_container_id;
	}

	const LumpId getLumpId(const ResourceId res_id) const
	{
		const ResourceRecord* res_rec = getResourceRecord(res_id);
		if (res_rec)
			return res_rec->mLumpId;

		static const LumpId invalid_lump_id = -1;
		return invalid_lump_id;
	}


	static const size_t MAX_RESOURCE_CONTAINERS = 255;
	std::vector<ResourceContainer*>	mContainers;
	ResourceContainerId				mTextureManagerContainerId;

	std::vector<FileAccessor*>		mAccessors;
	std::vector<std::string>		mResourceFileNames;
	std::vector<std::string>		mResourceFileHashes;

	// Map resource pathnames to ResourceIds
	typedef OHashTable<ResourcePath, ResourceIdList> ResourceIdLookupTable;
	ResourceIdLookupTable			mResourceIdLookup;


	// ---------------------------------------------------------------------------
	// Private helper functions
	// ---------------------------------------------------------------------------

	void openResourceFile(const OString& filename);

	bool visible(const ResourceId res_id) const;
};


// ============================================================================
//
// Externally visible functions
//
// ============================================================================

//
// Res_GetEngineResourceFileName
//
// Returns the file name for the engine's resource file. Use this function
// rather than hard-coding the file name.
//
static inline const OString& Res_GetEngineResourceFileName()
{
	static const OString& filename("ODAMEX.WAD");
	return filename;
}

void Res_OpenResourceFiles(const std::vector<std::string>& filename);
void Res_CloseAllResourceFiles();

const std::vector<std::string>& Res_GetResourceFileNames();
const std::vector<std::string>& Res_GetResourceFileHashes();


const ResourceId Res_GetResourceId(const OString& name, const OString& directory = global_directory_name);

const ResourceIdList Res_GetAllResourceIds(const OString& name, const OString& directory = global_directory_name); 

const OString& Res_GetLumpName(const ResourceId res_id);

const std::string& Res_GetResourceContainerFileName(const ResourceId res_id);


const ResourcePath& Res_GetResourcePath(const ResourceId res_id);

// ----------------------------------------------------------------------------
// Res_CheckLump
// ----------------------------------------------------------------------------

bool Res_CheckLump(const ResourceId res_id);

static inline bool Res_CheckLump(const OString& name, const OString& directory = global_directory_name)
{
	return Res_CheckLump(Res_GetResourceId(name, directory));
}


// ----------------------------------------------------------------------------
// Res_GetLumpLength
// ----------------------------------------------------------------------------

size_t Res_GetLumpLength(const ResourceId res_id);

static inline size_t Res_GetLumpLength(const OString& name, const OString& directory = global_directory_name)
{
	return Res_GetLumpLength(Res_GetResourceId(name, directory));
}


// ----------------------------------------------------------------------------
// Res_ReadLump
// ----------------------------------------------------------------------------

size_t Res_ReadLump(const ResourceId res_id, void* data);

static inline size_t Res_ReadLump(const OString& name, void* data)
{
	return Res_ReadLump(Res_GetResourceId(name), data);
}


// ----------------------------------------------------------------------------
// Res_CacheLump
// ----------------------------------------------------------------------------

void* Res_CacheLump(const ResourceId res_id, int tag);

static inline void* Res_CacheLump(const OString& name, int tag)
{
	return Res_CacheLump(Res_GetResourceId(name), tag);
}

bool Res_CheckMap(const OString& mapname);
const ResourceId Res_GetMapResourceId(const OString& lump_name, const OString& mapname);



#endif	// __RES_MAIN_H__
