/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "LogFile.h"
#include <string.h>
#include <sstream>
#include <thread>
#include <iomanip>

using namespace LOG;

const std::string LogFile::_debug = "debug";
const std::string LogFile::_info = "info";
const std::string LogFile::_warn = "warn";
const std::string LogFile::_fatal = "fatal";
const std::string LogFile::_other = "other";

void LogFile::write(const char *format, va_list args) 
   throw (FileOpenFailedException&, FileWriteFailedException&, std::invalid_argument)
{
    //get date
    time_t lnow;
    lnow = time(&lnow);
    struct tm lvtm;
    localtime_r(&lnow, &lvtm);

    //Lock
    std::lock_guard<std::mutex> llock(_mutex);

    //get the associated file handle of the filename
    if (     _fileHandle == nullptr ||
            (_fileHandle != nullptr && (lvtm.tm_year != _currentTM.tm_year ||
            lvtm.tm_mon != _currentTM.tm_mon ||
            lvtm.tm_mday != _currentTM.tm_mday))) {

        //Construct the new name
        std::ostringstream lsFilePathString;
        lsFilePathString << _fileDir << "/" << _prefixFileName << "_" << lvtm.tm_year+1900 << std::setw(2) << std::setfill('0') << lvtm.tm_mon+1 << std::setw(2) << std::setfill('0') << lvtm.tm_mday << ".log";
        std::string lsFilePath = lsFilePathString.str();

        //Close the older one if the older one is existing
        if (_fileHandle != nullptr) {
            fclose(_fileHandle);
            _fileHandle = nullptr;
        }

        //Create a new one
        _fileHandle = fopen(lsFilePath.c_str(), "a+w");

        //failed to create file the throw a exception
        if (_fileHandle == nullptr)
            throw FileOpenFailedException(lsFilePath);

        //Store the current file path name
        _currentFilePath = lsFilePath;

        //Copy the current time
        memcpy(&_currentTM, &lvtm, sizeof (tm));
    }
  
    //write the base information
    std::ostringstream lsAux ;
    lsAux << std::setw(2) << std::setfill('0') << lvtm.tm_hour << ":" << std::setw(2) << std::setfill('0') << lvtm.tm_min << ":"<< std::setw(2) << std::setfill('0') << lvtm.tm_sec<< "-TID:" << std::this_thread::get_id() <<" ";
    fprintf(_fileHandle, "%s", lsAux.str().c_str());
    //write the parameter information
    vfprintf(_fileHandle, format, args);
    //write the '\n'
    fprintf(_fileHandle, "\n");
	fflush(_fileHandle);
    //done
    if (ferror(_fileHandle))
        throw FileWriteFailedException(_currentFilePath);
}