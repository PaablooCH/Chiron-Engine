#include "Pch.h"
#include "Folder.h"

#include "Modules/ModuleFileSystem.h"

#include "DataModels/FileSystem/UID/UIDGenerator.h"

Folder::Folder(const std::string& path) : _uid(Chiron::UIDGenerator::GenerateUID()), _name(path), _path(path), _parent(nullptr),
_opened(false)
{
}

Folder::Folder(const std::string& path, Folder* parent) : _uid(Chiron::UIDGenerator::GenerateUID()), 
_name(ModuleFileSystem::GetFile(path.c_str())), _parent(parent), _opened(false)
{
    _parent->LinkSubdirectory(this);
}

Folder::~Folder()
{
}

Folder* Folder::FindFolder(UID uid)
{
    std::queue<Folder*> queue;
    queue.push(this);
    while (!queue.empty())
    {
        Folder* folder = queue.front();
        queue.pop();
        if (folder->GetUID() == uid)
        {
            return folder;
        }

        for (auto& subdirectory : folder->GetSubdirectories())
        {
            queue.push(subdirectory.get());
        }
    }
    return nullptr;
}

void Folder::LinkSubdirectory(Folder* subdirectory)
{
    assert(subdirectory);

    if (!IsSubdirectory(subdirectory))
    {
        subdirectory->_parent = this;
        std::string newPath = _path + '/' + subdirectory->_name + '/';
        subdirectory->_path = newPath;
        _subdirectories.push_back(std::unique_ptr<Folder>(subdirectory));
    }
}

Folder* Folder::UnlinkSubdirectory(Folder* subdirectory)
{
    assert(subdirectory);
    if (IsSubdirectory(subdirectory))
    {
        auto childIt = std::ranges::find_if(_subdirectories,
            [subdirectory](std::unique_ptr<Folder>& actualChild)
            {
                return actualChild.get() == subdirectory;
            });

        auto orphan = childIt->release();
        _subdirectories.erase(childIt);

        return orphan;
    }
    return nullptr;
}

bool Folder::IsSubdirectory(Folder* subdirectory)
{
    return std::ranges::any_of(_subdirectories.begin(), _subdirectories.end(),
        [subdirectory](std::unique_ptr<Folder>& actualChild)
        {
            return actualChild.get() == subdirectory;
        });
}

void Folder::SetParent(Folder* parent)
{
    if (parent->IsSubdirectory(this))
    {
        return;
    }
    std::string newPath = parent->_path + '/' + _name;
    if (ModuleFileSystem::MoveDirectory(_path.c_str(), newPath.c_str()))
    {
        std::ignore = _parent->UnlinkSubdirectory(this);
        parent->LinkSubdirectory(this);
    }
}