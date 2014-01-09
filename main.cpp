//
//  main.cpp
//  AssignPic
//
//  Created by Donghoon Choi on 12-10-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stack>
#include "exif.h"

#define MAX_ARRAY (10)
#define MAX_PATH (1024)
#define MAX_PIC (4)
#define MAX_MOV (5)

//#define __FOR_TEST_IN_MAC

std::string g_str_pic_ext[MAX_PIC] = {".jpg", ".png", ".gif", ".bmp"};
std::string g_str_mov_ext[MAX_MOV] = {".avi", ".mov", ".mpg", ".mp4", ".k3g"};

int GetFileList(std::stack<std::string*>*, std::string*);
void BackupFile(std::string*);
bool IsPicture(std::string*);
bool IsVideo(std::string*);
bool IsThereSameFile(const char* pDestDir, std::string* pSrc);
std::string IsThereSameNamFile(const char* pDestDir, std::string* pSrc);
void GetPicDate(int*, int*, int*, std::string*);
void MoveFile(const char*, std::string*);

int main()
{
    std::stack<std::string*>* pFileList = new std::stack<std::string*>;
    std::string* pSrcDir = new std::string;
#ifdef __FOR_TEST_IN_MAC
    (*pSrcDir) = "/Users/choidonghoon/Downloads/test";
#else
    (*pSrcDir) = "/inven/share/unsorted_pic_mov";
#endif    
    
    int iRet = -1;
    while(iRet == -1) {
        iRet = GetFileList(pFileList, pSrcDir);
        
        while(pFileList->size() != 0) {
            std::string* pFileName = pFileList->top();
            printf("PROC: %s\n", pFileName->data());
            BackupFile(pFileName);
            pFileList->pop();
            delete pFileName;
        }
	}
    
    delete pSrcDir;    
    delete pFileList;
    printf("Mission Complete!!\n");
    
    return 0;
}

int GetFileList(std::stack<std::string*>* pList, std::string* pCurrDirFullPath) {
    
    DIR            *dir_info;
    struct dirent  *dir_entry;
    
    dir_info = opendir(pCurrDirFullPath->c_str());
    if (!dir_info)
    {
        printf("opendir fail at %s\n", pCurrDirFullPath->c_str());
        return 0;
        
    }
    
    
    struct stat* p_statBuff = new struct stat;
    for(dir_entry = readdir(dir_info); dir_entry; dir_entry = readdir(dir_info)) {
        if(dir_entry->d_ino == 0) continue;
        if(strcmp(dir_entry->d_name, ".") == 0) continue;
        if(strcmp(dir_entry->d_name, "..") == 0) continue;
        
        std::string strFullPath = (*pCurrDirFullPath);
        strFullPath += "/";
        strFullPath += dir_entry->d_name;
        int ret = lstat(strFullPath.c_str(), p_statBuff);
        
        if ( S_ISDIR(p_statBuff->st_mode) ) {
            ret = GetFileList(pList, &strFullPath);
            
            if(ret < 0) {
                delete p_statBuff;
                closedir(dir_info);
                return ret;
            } 
        }
        else if(IsVideo(&strFullPath)||IsPicture(&strFullPath)) {
            std::string* p_path = new std::string;
            (*p_path) = strFullPath;
            pList->push(p_path);
            
            if(pList->size() >= MAX_ARRAY) {
                delete p_statBuff;
                closedir(dir_info);
                return -1;
            }
        }
    }
    
    delete p_statBuff;
    closedir(dir_info);
    return 0;
}

void BackupFile(std::string* pFileName) {
    int iLen = pFileName->length();
    if(iLen < 3 ) return;
    
    if(IsPicture(pFileName)) {
        int iDay = 0;
        int iMonth = 0;
        int iYear = 0;
        GetPicDate(&iYear, &iMonth, &iDay, pFileName);
        char DestDir[MAX_PATH];
        memset(DestDir, 0, MAX_PATH);
        
        
#ifdef __FOR_TEST_IN_MAC
        sprintf(DestDir, "/Users/choidonghoon/Downloads/test1/%04d", iYear);
        mkdir(DestDir, 0777);
        sprintf(DestDir, "/Users/choidonghoon/Downloads/test1/%04d/%02d", iYear, iMonth);
        mkdir(DestDir, 0777);
        sprintf(DestDir, "/Users/choidonghoon/Downloads/test1/%04d/%02d/%02d", iYear, iMonth, iDay);
        mkdir(DestDir, 0777);
#else
        sprintf(DestDir, "/inven/share/memories/pic/%04d", iYear);
        mkdir(DestDir, 0777);
        sprintf(DestDir, "/inven/share/memories/pic/%04d/%02d", iYear, iMonth);
        mkdir(DestDir, 0777);
        sprintf(DestDir, "/inven/share/memories/pic/%04d/%02d/%02d", iYear, iMonth, iDay);
        mkdir(DestDir, 0777);
#endif    
        
        MoveFile(DestDir, pFileName);
    }
    else if(IsVideo(pFileName)) {
#ifdef __FOR_TEST_IN_MAC
        std::string strDestMovDir = "/Users/choidonghoon/Downloads/test1";
#else
        std::string strDestMovDir = "/inven/share/memories/mov";
#endif    
        
        if(IsThereSameFile(strDestMovDir.data(), pFileName)) {
            char szCmd[MAX_PATH];
            sprintf(szCmd, "rm \"%s\"", pFileName->data());
            system(szCmd);
        }
        else MoveFile(strDestMovDir.c_str(), pFileName);
    }
}

bool IsPicture(std::string* pFileName) {
    size_t len = pFileName->length();
    if(len <= 4) return false;
    size_t pos_ext = len - 4;
    
    std::string filename = *pFileName;
    std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
    
    for(int i=0;i<MAX_PIC;i++)
        if(filename.rfind(g_str_pic_ext[i].c_str()) == pos_ext) return true;
     
    return false;
}

bool IsVideo(std::string* pFileName) {
    size_t len = pFileName->length();
    if(len <= 4) return false;
    size_t pos_ext = len - 4;
    
    std::string filename = *pFileName;
    std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
    
    for(int i=0;i<MAX_MOV;i++)
        if(filename.rfind(g_str_mov_ext[i].c_str()) == pos_ext) return true;
    
    return false;
}

bool IsThereSameFile(const char* pDestDir, std::string* pSrc) {
    std::string FullDestPath = IsThereSameNamFile(pDestDir, pSrc);
    if(FullDestPath.length() <= 0) return false;
    
    struct stat* p_statDest = new struct stat;
    int ret = lstat(FullDestPath.data(), p_statDest);
    if(ret != 0)
    {
        delete p_statDest;
        return false;
    }
    /*
    if ( _stat(filename, &buf) != 0 ) {
        switch (errno) {
            case ENOENT:
                fprintf(stderr, "File %s not found.\n", filename); break;
            case EINVAL:
                fprintf(stderr, "Invalid parameter to _stat.\n"); break;
            default:
                fprintf(stderr, "Unexpected error in _stat.\n");
        }
    }
     */
    
    struct stat* p_statSrc = new struct stat;
    ret = lstat(pSrc->data(), p_statSrc);
    if(ret != 0)
    {
        delete p_statDest;
        delete p_statSrc;
        return false;
    }
    
    if((p_statSrc->st_ctime != p_statDest->st_ctime) ||
       (p_statSrc->st_size != p_statDest->st_size)) 
    {
        delete p_statDest;
        delete p_statSrc;
        return false;
    }
    
    delete p_statDest;
    delete p_statSrc;
    return true;
}

std::string IsThereSameNamFile(const char* pDestDir, std::string* pSrc) {
    size_t found;
    found = pSrc->rfind("/");
    std::string filename = pSrc->substr(found+1);
    
    std::string FullDestPath;
    FullDestPath = pDestDir;
    FullDestPath += "/";
    FullDestPath += filename;
    
    FILE* pFile = fopen(FullDestPath.data(), "r");
    if(pFile) fclose(pFile);
    else FullDestPath = "";    
    return FullDestPath;    
}

void GetPicDate(int* pYear, int* pMonth, int* pDay, std::string* pFile) {
    (*pYear) = 0;
    (*pMonth) = 0;
    (*pDay) = 0;
    size_t len = pFile->length();
    if(len <= 4) return;
    size_t pos_ext = len - 4;
    
    bool bSetExif = false;
    
    std::string filename = *pFile;
    std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
    
    if(filename.rfind(g_str_pic_ext[0]) == pos_ext) {
        FILE *fp = fopen(pFile->data(), "rb");
        if(fp) {
            fseek(fp, 0, SEEK_END);
            unsigned long fsize = ftell(fp);
            rewind(fp);
            unsigned char *buf = new unsigned char[fsize];
            if(fread(buf, 1, fsize, fp) == fsize) {
                EXIFInfo result;
                if(ParseEXIF(buf, fsize, result) == 0) {
                    if(result.dateTimeOriginal) {
                        sscanf(result.dateTimeOriginal, "%d:%d:%d", pYear, pMonth, pDay);
                        if((*pYear) > 2000) bSetExif = true;
                    }
                    
                    if(bSetExif == false && result.dateTimeModified) {
                        sscanf(result.dateTimeModified, "%d:%d:%d", pYear, pMonth, pDay);
                        if((*pYear) > 2000) bSetExif = true;
                    }
                }
            }
            fclose(fp);
            delete[] buf;
        }
    }
    
    if(!bSetExif) {
        struct stat statFile;
        int ret = lstat(pFile->data(), &statFile);
        if(ret != 0) return;
        
        struct tm* t = localtime(&statFile.st_mtime);
        
        (*pYear) = t->tm_year + 1900;
        (*pMonth) = t->tm_mon + 1;
        (*pDay) = t->tm_mday;
        
        if((*pYear) < 1960) {
            t = localtime(&statFile.st_ctime);
            
            (*pYear) = t->tm_year + 1900;
            (*pMonth) = t->tm_mon + 1;
            (*pDay) = t->tm_mday;
        }
    }
}

void MoveFile(const char* pDestDir, std::string* pSrc) {
    std::string srcPath = *pSrc;
    std::string destPath;
    for(destPath = IsThereSameNamFile(pDestDir, &srcPath); destPath.length() != 0; destPath = IsThereSameNamFile(pDestDir, &srcPath)) {
        size_t found;
        found = srcPath.rfind("/");
        std::string dir = srcPath.substr(0, found+1);
        std::string file = srcPath.substr(found+1);
        found = file.rfind(".");
        std::string filename = file.substr(0, found);
        std::string ext = file.substr(found);
        filename += "1";
        srcPath = dir + filename + ext;
    }
    
    std::string file = pDestDir;
    
    size_t found;
    found = srcPath.rfind("/");
    file += srcPath.substr(found);
    
    char szCmd[MAX_PATH];
    memset(szCmd, 0, MAX_PATH);
    sprintf(szCmd, "mv \"%s\" \"%s\"", pSrc->data(), file.data());
    printf("move : %s -> %s\n", pSrc->data(), file.data());
    system(szCmd);
}

