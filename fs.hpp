#ifndef NEXTLIB_FS_HPP
#define NEXTLIB_FS_HPP

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#include <dirent.h>
#define GetCurrentDir getcwd
#endif

#include <string>
#include <vector>
#include <fstream>
#include <nextlib/error.hpp>

class File {
public:
    File() {}

    File(std::string filename) {
        this->filename = filename;
    }

    void Open(std::string filename) {
        this->filename = filename;
    }

    std::vector<std::string> ReadAllLines() {
        std::vector<std::string> lines;
        std::string temp;
        std::ifstream file;
        file.open(filename);
        while (getline(file, temp)) {
            lines.push_back(temp);
        }
        file.close();
        return lines;
    }

    std::string ReadAllText() {
        std::string content = "", temp;
        std::ifstream file;
        file.open(filename);
        while (getline(file, temp)) {
            content += temp + "\n";
        }
        file.close();
        content = content.substr(0, content.size() -1);
        return content;
    }

    void WriteAllLines(std::vector<std::string> lines) {
        std::string write = "";
        for (auto &line : lines) {
            write += line + "\n";
        }
        WriteAllText(write);
    }

    void WriteAllText(std::string text) {
        std::ofstream file;
        try {
            file.open(filename);
            file << text;
            file.close();
        } catch (const std::ofstream::failure& e) {
            throw (Exception("Failed to write text into file."));
        }
    }

    void AppendText(std::string text) {
        std::fstream file(filename, std::ios::in | std::ios::out | std::ios::app);
        try {
            file << text;
            file.close();
        } catch (const std::fstream::failure& e) {
            throw (Exception("Failed to append text to file."));
        }
    }
private:
    std::string filename;
};

class Directory {
public:
    Directory() {}

    Directory(std::string dirname) {
        this->dirname = dirname;
    }

    void Open(std::string dirname) {
        this->dirname = dirname;
    }

    std::string GetCurrent() {
        char buff[FILENAME_MAX];
        GetCurrentDir(buff, FILENAME_MAX);
        return std::string(buff);
    }

    std::vector<std::string> GetFiles() {
        std::vector<std::string> files;
        #ifdef _WIN32
        std::string searchPath = dirname + "/*.*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = ::FindFirstFile(searchPath.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    files.push_back(fd.cFileName);
                }
            } while (::FindNextFile(hFind, &fd));
            ::FindClose(hFind);
        }
        #else
        DIR* dir = opendir(std::string(GetCurrent() + "/" + dirname).c_str());
        if (dir == NULL) {
            dir = opendir(dirname.c_str());
            if (dir == NULL) {
                throw (Exception("Folder don't exists."));
            }
        }

        struct dirent* entity;
        entity = readdir(dir);
        while (entity != NULL) {
            if (entity->d_type != DT_DIR)
                files.push_back(std::string(entity->d_name));
            entity = readdir(dir);
        }

        closedir(dir);
        #endif
        return files;
    }

    std::vector<std::string> GetDirectories() {
        std::vector<std::string> dirs;

        #ifdef _WIN32
        HANDLE hFile;
        std::string strFilePath;
        std::string strPattern;
        WIN32_FIND_DATA FileInformation;

        strPattern = GetCurrent() + "\\" + dirname + "\\*.*";
        hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);

        bool errorOccured = true;

        if (hFile != INVALID_HANDLE_VALUE) {
            errorOccured = false;
        } else {
            strPattern = dirname + "\\*.*";
            hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
            if (hFile != INVALID_HANDLE_VALUE) {
                errorOccured = false;
            }
        }

        if (!errorOccured) {
            do {
                if (FileInformation.cFileName[0] != '.') {
                    strFilePath.erase();
                    strFilePath = dirname + "\\" + FileInformation.cFileName;
                    if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        int slashes = 0, tempsl = 0;
                        std::string tempst = "";
                        for (int i = 0; i < strFilePath.length(); i++) {
                            if (strFilePath[i] == '\\') slashes++;
                        }
                        for (int i = 0; i < strFilePath.length(); i++) {
                            if (strFilePath[i] == '\\') tempsl++;
                            else if (tempsl == slashes) {
                                tempst += strFilePath[i];
                            }
                        }
                        dirs.push_back(tempst);
                    }
                }
            } while(::FindNextFile(hFile, &FileInformation) == TRUE);
        }
        #else
        DIR* dir = opendir(std::string(GetCurrent() + "/" + dirname).c_str());
        if (dir == NULL) {
            dir = opendir(dirname.c_str());
            if (dir == NULL) {
                throw (Exception("Folder don't exists."));
            }
        }

        struct dirent* entity;
        entity = readdir(dir);
        while (entity != NULL) {
            if (entity->d_type == DT_DIR && std::string(entity->d_name) != "." && std::string(entity->d_name) != "..")
                dirs.push_back(std::string(entity->d_name));
            entity = readdir(dir);
        }

        closedir(dir);
        #endif

        return dirs;
    }
private:
    std::string dirname;
};

#endif // NEXTLIB_FS_HPP