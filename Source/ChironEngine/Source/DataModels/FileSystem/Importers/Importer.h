#pragma once

template<typename U>
class Importer
{
public:
    Importer() = default;
    virtual ~Importer() {}

    // Reads a path that indicates a file to import to engine
    virtual void Import(const char* filePath, const std::shared_ptr<U>& resource) = 0;
    // Reads binary and generates your own data
    virtual void Load(const char* fileBuffer, const std::shared_ptr<U>& resource) = 0;

protected:
    // Reads your own data and generates a file buffer
    virtual void Save(const std::shared_ptr<U>& resource) = 0;
};
