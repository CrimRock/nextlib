#ifndef NEXTLIB_FS_HPP
#define NEXTLIB_FS_HPP

#ifdef __unix__
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#define GetCurrentDir getcwd

#elif defined(_WIN32)
#include <direct.h>
#include <windows.h>
#define GetCurrentDir _getcwd

#else
#error "Platform not supported."
#endif

#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <nextlib/error.hpp>

namespace Next
{
    class File {
    public:
        File() {}

        File(std::string filename) {
            this->filename = filename;
        }

        void Open(std::string filename) {
            this->filename = filename;
        }

        static void Delete(std::string filename) {
            if (std::remove(filename.c_str()) != 0) {
                throw (Exception("Failed to delete file."));
            }
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

	    static void Create(std::string dirname) {
#ifdef __unix__
		    mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
		    _mkdir(dirname.c_str());
#endif
	    }

        static int Delete(std::string dirname, bool recursive = true) {
            int IRC;
#ifdef __unix__
	        IRC = LinuxDeleteDirectory(dirname, recursive);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            IRC = WinDeleteDirectory(dirname, recursive);
#endif
            return IRC;
        }

        static bool Exists(std::string dirname) {
#ifdef __unix__
            if (dirname == "") return false;

            DIR *dir;

            dir = opendir(dirname.c_str());

            if (dir != NULL) {
                closedir(dir);
                return true;
            } else {
                return false;
            }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            DWORD ftyp = GetFileAttributesA(dirname.c_str());
            if (ftyp == INVALID_FILE_ATTRIBUTES)
                return false;
            
            if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
                return true;
            
            return false;
#endif
        }

        static std::string GetCurrent() {
            char buff[FILENAME_MAX];
            GetCurrentDir(buff, FILENAME_MAX);
            return std::string(buff);
        }

        std::vector<std::string> GetFiles() {
            std::vector<std::string> files;
#ifdef __unix__
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
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
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
#endif
            return files;
        }

        std::vector<std::string> GetDirectories() {
            std::vector<std::string> dirs;

#ifdef __unix__
            DIR* dir = opendir(std::string(GetCurrent() + "/" + dirname).c_str());
            if (dir == NULL) {
                dir = opendir(dirname.c_str());
                if (dir == NULL) {
                    throw (Exception("Folder not found."));
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
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
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
            } else {
                throw (Exception("Folder not found."));
            }
#endif

            return dirs;
        }
    private:
        std::string dirname;

#ifdef __unix__
        static int LinuxDeleteDirectory(std::string dirname, bool recursive) {
            DIR *dir;
            struct dirent *entry;
            char path[PATH_MAX];
            char dirn[dirname.length()];
            strcpy(dirn, dirname.c_str());

            if (path == NULL) {
                // Out of memory error
                return 1;
            }
            dir = opendir(dirname.c_str());
            if (dir == NULL) {
                // Directory not exists
                return -1;
            }

            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
                    snprintf(path, (size_t) PATH_MAX, "%s/%s", dirn, entry->d_name);
                    if (entry->d_type == DT_DIR) {
                        LinuxDeleteDirectory(std::string(path), recursive);
                    } else {
                        File::Delete(std::string(path));
                    }
                }
            }
            closedir(dir);
            remove(dirname.c_str());

            return 0;
        }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        static int WinDeleteDirectory(std::string dirname, bool bDeleteSubdirectories) {
            bool bSubdirectory = false;

            HANDLE hFile;
            std::string strFilePath;
            std::string strPattern;
            WIN32_FIND_DATA FileInformation;

            strPattern = dirname + "\\*.*";
            hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
            if (hFile != INVALID_HANDLE_VALUE) {
                do {
                    if (FileInformation.cFileName[0] != '.') {
                        strFilePath.erase();
                        strFilePath = dirname + "\\" + FileInformation.cFileName;

                        if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                            if (bDeleteSubdirectories) {
                                int iRC = WinDeleteDirectory(strFilePath, bDeleteSubdirectories);
                                if (iRC)
                                    return iRC;
                            } else {
                                bSubdirectory = true;
                            }
                        } else {
                            if (::SetFileAttributes(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE) {
                                return ::GetLastError();
                            }

                            if (::DeleteFile(strFilePath.c_str()) == FALSE) {
                                return ::GetLastError();
                            }
                        }
                    }
                } while (::FindNextFile(hFile, &FileInformation) == TRUE);

                ::FindClose(hFile);
                DWORD dwError = ::GetLastError();
                if (dwError != ERROR_NO_MORE_FILES) {
                    return dwError;
                } else {
                    if (!bSubdirectory) {
                        if (::SetFileAttributes(dirname.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE) {
                            return ::GetLastError();
                        }

                        if (::RemoveDirectory(dirname.c_str()) == FALSE) {
                            return ::GetLastError();
                        }
                    }
                }
            }

            return 0;
        }
#endif
    };
}

#endif // NEXTLIB_FS_HPP
