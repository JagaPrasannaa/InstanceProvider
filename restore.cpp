
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <Windows.h>
#include <conio.h>
#include <codecvt>
#include <locale>
#include <unordered_map>

/*
    WSTRING TO STRING AND STRING TO WSTRING CONVERSION
*/
using convert_t = std::codecvt_utf8<wchar_t>;
std::wstring_convert<convert_t, wchar_t> strconverter;
std::string to_string(std::wstring wstr)
{
    return strconverter.to_bytes(wstr);
}

std::wstring to_wstring(std::string str)
{
    return strconverter.from_bytes(str);
}

struct SuccessReport
{
    std::string filePath;
    std::string operation;
};
struct FailureReport
{
    std::string filePath;
    std::string operation;
    int errorcode;
};
enum Type {
    FILE_TYPE = 1,
    DIRECTORY = 2,
    DIRECTORY_FILE = 3
};

/*REVERT FUNCTION DECRATION*/
int CreateDirRevert(
    std::string filePath,
    std::set<std::string>* CreateDirSet,
    std::set<std::string>* CreateFileSet,
    std::set<std::string>* DeleteDirSet,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet
);
int DeleteDirRevert(
    std::string filePath,
    std::set<std::string>* CreateFileSet,
    std::set<std::string>* DeleteDirSet,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet
);
int CreateFileRevert(
    std::string filePath,
    std::set<std::string>* CreateFileSet,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet
);
int DeleteFileRevert(
    std::string file,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet
);
int WriteRevert(
    std::string file,
    std::set<std::string>* WriteFileSet
);
/*REVERT DECARATION ENDS*/

/*UTILS FOR RESTORE AND DELETE DIR*/
bool dirExists(const std::string& dirName_in);
bool CopyDirTo(const std::wstring& source_folder, const std::wstring& target_folder);
int file_check(std::string filepath,std::set<std::string>* fileSet,Type type);
int DeleteDirectory(const std::string& refcstrRootDirectory,bool bDeleteSubdirectories);
int ClearRecord(std::string rootFile,std::set<std::string>* FileSet,Type type,std::string opr,std::string status,std::string cmd);
int RestoreContent(std::string src, std::string desc, Type type);
void printSet(std::set<std::string> myset);
/*UTILS FOR RESTORE ENDS*/

/*Global Decalration*/
std::string g_fp;
int s_id = 1;
int f_id = 1;
//auto cmp_success = [](SuccessReport lhs, SuccessReport rhs) { return lhs.operation.length() > rhs.operation.length(); };
//auto cmp_failure = [](FailureReport lhs, FailureReport rhs) { return lhs.operation.length() > rhs.operation.length(); };
std::map<int, struct SuccessReport> process_success;
std::map<int, struct FailureReport> process_failure;
int main()
{
    /*  SuspiciousProcessData spdObj = baseDataManager.getSuspeciesData(S_GUID);
        RestoreHandlerMeta rhmObj = baseBackupManager.InitRestoreHandler(spd.GetCheckPointGUID())
        string g_fp = rhmObj.getFolerPoint();
        std::set<std::string> g_DeleteDirSet;//Delete dir
        std::set<std::string> g_CreateFileSet = spdObj.getFileList(3);//create
        std::set<std::string> g_DeleteFileSet = spdObj.getFileList();
        std::set<std::string> g_WriteFileSet  = spdObj.getFileList(4);//write files
        std::set<std::string> g_CreateDirSet  = spdObj.getFileList();//create dir
        
    */
    g_fp = "P:\\data\\backup\\P";
    std::set<std::string> g_DeleteDirSet = {};
    std::set<std::string> g_CreateFileSet = {};//create files
    std::set<std::string> g_DeleteFileSet = { "P:\\data\\working\\abcde\\","P:\\data\\working\\abcde\\abc.txt","P:\\data\\working\\init.xlsx","P:\\data\\working\\content.txt" };//delete files
    std::set<std::string> g_WriteFileSet = { "P:\\data\\working\\content.txt" };//write files
    std::set<std::string> g_CreateDirSet = { "P:\\data\\working\\attack" };//create dir
    int res=0;
    std::string file_in_set;
    //Create-Dir-Revert
    for (auto itr = g_CreateDirSet.begin(); itr!= g_CreateDirSet.end();) {//iterate create list with var file
        file_in_set = *itr;
        std::cout << "Processing : " << file_in_set << std::endl;
        CreateDirRevert(file_in_set, &g_CreateDirSet, &g_CreateFileSet, &g_DeleteDirSet, &g_DeleteFileSet, &g_WriteFileSet);
        itr = g_CreateDirSet.begin();
    }
    //Delete dir Revert
    for (auto itr = g_DeleteFileSet.begin(); itr != g_DeleteFileSet.end();) {
        file_in_set = *itr;
        std::cout << "Processing : " << file_in_set << std::endl;
        res = DeleteDirRevert(file_in_set, &g_CreateFileSet, &g_DeleteDirSet, &g_DeleteFileSet, &g_WriteFileSet);
        if (res == 1) {
            itr = g_DeleteFileSet.begin();
        }
        else {
            ++itr;
        }
    }
    //Create-File-Revert
    for (auto itr = g_CreateFileSet.begin(); itr != g_CreateFileSet.end();) {
        file_in_set = *itr;
        CreateFileRevert(file_in_set, &g_CreateFileSet, &g_DeleteFileSet, &g_WriteFileSet);
        itr = g_CreateFileSet.begin();
    }
    //At this stage no files that the attacker created will not be available g_WriteList this reduces the write-revert opr
    //Delete-Revert only the files that user created
    for (auto itr = g_DeleteFileSet.begin(); itr != g_DeleteFileSet.end();) {
        file_in_set = *itr;
        std::cout << "Processing : " << file_in_set << std::endl;
        DeleteFileRevert(file_in_set, &g_DeleteFileSet, &g_WriteFileSet);
        itr = g_DeleteFileSet.begin();
    }
    //dirty-Revert only the files that user stored
    for (auto itr = g_WriteFileSet.begin(); itr != g_WriteFileSet.end();) {
        file_in_set = *itr;
        WriteRevert(file_in_set, &g_WriteFileSet);
        itr = g_WriteFileSet.begin();
    }
    std::cout << "status success : " << std::endl;
    for (int i = 1; i < s_id; i++) {
        SuccessReport sr = process_success[i];
        std::cout << "filePath :" << sr.filePath << " Operation: " << sr.operation << std::endl;
    }
    std::cout << "Failure" << "\n" << std::endl;
    for (int i = 1; i < f_id; i++) {
        FailureReport fr = process_failure[i];
        std::cout << "filePath :" << fr.filePath << " Operation: " << fr.operation << std::endl;
    }
}


int CreateDirRevert(
    std::string filePath,
    std::set<std::string>* CreateDirSet,
    std::set<std::string>* CreateFileSet,
    std::set<std::string>* DeleteDirSet,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet
) {
    //filePath -> root Path created by attacker
    if (file_check(filePath, DeleteFileSet, Type::FILE_TYPE)==1) {//check the file in spdObj delete list
        if (file_check(filePath, NULL, Type::DIRECTORY)==1) {//check the file in folderpoint(backdir path)P:\data\backup\P\data\working
            //file exist in back up so restore
            RestoreContent(g_fp,filePath,Type::DIRECTORY);//src,desc,type
            ClearRecord(filePath, CreateDirSet, Type::DIRECTORY, "restore", "success","store"); //opr->clearing root and subdirecotries in createdirset
            //ClearRecord(filePath, DeleteDirSet, Type::DIRECTORY, "restore", "success");
            ClearRecord(filePath, CreateFileSet, Type::DIRECTORY, "restore", "success","store");
            ClearRecord(filePath, DeleteFileSet, Type::DIRECTORY, "restore", "success","store");
            ClearRecord(filePath, WriteFileSet, Type::DIRECTORY, "restore", "success","no_store");
        }
    }
    else {
        std::cout << "File does not deleted by attacker and doesnt exist in backup so manual delete" << std::endl;
        DeleteDirectory(filePath,true);
        ClearRecord(filePath, CreateDirSet, Type::DIRECTORY, "delete", "success","store"); //opr->clearing root and subdirecotries in createdirset
        std::cout << "After Create Dir Set";
        printSet(*CreateDirSet);
        std::cout << "\n" << std::endl;
        //ClearRecord(filePath, DeleteDirSet, Type::DIRECTORY, "delete", "success"); //opr->clearing root and subdirectories in deletedir set
        std::cout << "After Delete Dir Set";
        printSet(*DeleteDirSet);
        std::cout << "\n" << std::endl;
        ClearRecord(filePath, CreateFileSet, Type::DIRECTORY, "delete", "success","store");//opr->clearing files created inside dir and subdirecotries in CreateFileSet
        std::cout << "After Create File Set";
        printSet(*CreateFileSet);
        std::cout << "\n" << std::endl;
        ClearRecord(filePath, DeleteFileSet, Type::DIRECTORY, "delete", "success","store");//opt->clearing files deleted inside dir and subdirecotries in DeleteFileSet
        std::cout << "After Delete File Set";
        printSet(*DeleteFileSet);
        std::cout << "\n" << std::endl;
        ClearRecord(filePath, WriteFileSet, Type::DIRECTORY, "delete", "success","no_store"); //opr->clearing files written inside dir and subdirecotries in WriteFileSet
        std::cout << "After Write File Set";
        printSet(*WriteFileSet);
        std::cout << "\n" << std::endl;
    }
    /*
        Clear CreateRevertdir records in other sets
        -> Directories created inside the dir(filePath) sub-folders in CreateDirSet
        -> Delete Dir and in it's sub-folders record in DeleteDirSet
        -> files created inside the dir and in it's sub-folders record in CreateFileSet
        -> files deleted inside the dir and in it's sub-folders record in DeleteFileSet
        -> files written inside the dir and in it's sub-folders record in WriteFileSet
    */

    return 1;
}
int DeleteDirRevert(
    std::string filePath,
    std::set<std::string>* CreateFileSet,
    std::set<std::string>* DeleteDirSet,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet) {
    if (file_check(filePath, NULL, Type::DIRECTORY) == 1) {
        RestoreContent(g_fp, filePath , Type::DIRECTORY);
        std::cout << "Restoring content" << std::endl;
        //ClearRecord(filePath, DeleteDirSet, Type::DIRECTORY, "restore", "success"); //opr->clearing root and subdirectories in deletedir set
        ClearRecord(filePath, CreateFileSet, Type::DIRECTORY, "restore", "success","store");//opr->clearing files created inside dir and subdirecotries in CreateFileSet
        ClearRecord(filePath, DeleteFileSet, Type::DIRECTORY,"restore", "success","store");//opt->clearing files deleted inside dir and subdirecotries in DeleteFileSet
        ClearRecord(filePath, WriteFileSet, Type::DIRECTORY, "restore", "success","no_store"); //opr->clearing files written inside dir and subdirecotries in WriteFileSet
    }
    else if (file_check(filePath, NULL, Type::DIRECTORY_FILE) == 1) {
        return 0;
    }
    else {
        std::cout << "Failed to recover the content : " << filePath << std::endl;
        ClearRecord(filePath, CreateFileSet, Type::DIRECTORY, "restore", "fail","store");//opr->clearing files created inside dir and subdirecotries in CreateFileSet
        ClearRecord(filePath, DeleteFileSet, Type::DIRECTORY, "restore", "fail","store");//opt->clearing files deleted inside dir and subdirecotries in DeleteFileSet
        ClearRecord(filePath, WriteFileSet, Type::DIRECTORY, "restore", "fail","no_store"); //opr->clearing files written inside dir and subdirecotries in WriteFileSet
    }
    std::cout << "DeleteDirRevert" << std::endl;
    printSet(*CreateFileSet);
    printSet(*DeleteFileSet);
    printSet(*WriteFileSet);
    return 1;
}


int CreateFileRevert(
    std::string filePath,
    std::set<std::string>* CreateFileSet,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet
) {//files created onlhy in user dir....(not in Attacker's dir)

    if (file_check(filePath, DeleteFileSet, Type::FILE_TYPE)) {//check the file in spdObj delete list
        if (file_check(filePath, NULL, Type::DIRECTORY_FILE)) {//check the file in folderpoint(backdir path)
            RestoreContent(g_fp,filePath,Type::FILE_TYPE);
            ClearRecord(filePath, CreateFileSet, Type::FILE_TYPE, "restore", "success","store");
            ClearRecord(filePath, DeleteFileSet, Type::FILE_TYPE, "restore", "success","store");
            ClearRecord(filePath, WriteFileSet, Type::FILE_TYPE, "restore", "success","no_store");
        }
    }
    else {
        remove(filePath.c_str());
        ClearRecord(filePath, CreateFileSet, Type::FILE_TYPE, "delete", "success","store");
        ClearRecord(filePath, DeleteFileSet, Type::FILE_TYPE, "delete", "success","store");
        ClearRecord(filePath, WriteFileSet, Type::FILE_TYPE, "delete", "success","no_store");
    }
    return 1;
}

int DeleteFileRevert(
    std::string file,
    std::set<std::string>* DeleteFileSet,
    std::set<std::string>* WriteFileSet) {
    if (file_check(file, NULL, Type::DIRECTORY_FILE) == 1) {//check the file in backup
        //Restore(file, g_fp);//restore the file from backup
        RestoreContent(g_fp, file, Type::FILE_TYPE);
        ClearRecord(file, DeleteFileSet, Type::FILE_TYPE, "restore", "success","store");
        ClearRecord(file, WriteFileSet, Type::FILE_TYPE, "restore", "success","no_store");
    }
    else {
        ClearRecord(file, DeleteFileSet, Type::FILE_TYPE, "restore", "failed", "store");
        ClearRecord(file, WriteFileSet, Type::FILE_TYPE, "restore", "failed", "no_store");
    }
    return 1;
}


int WriteRevert(std::string file,std::set<std::string> *WriteFileSet) {
    //delete(file);
    //Restore(File, g_fp);
    if (file_check(file, NULL, Type::DIRECTORY_FILE)) {
        remove(file.c_str());
        RestoreContent(g_fp, file, Type::FILE_TYPE);
        ClearRecord(file, WriteFileSet, Type::FILE_TYPE, "restore", "success","store");
    }
    else {
        ClearRecord(file, WriteFileSet, Type::FILE_TYPE, "restore", "failure","store");
    }
    return 1;
}

/*CLEARS THE FILE PATH IN THE SPECIFIED SET*/
int ClearRecord(std::string rootFile, std::set<std::string>* FileSet, Type type,std::string opr,std::string status,std::string cmd) {
    std::string FilePath;
    switch (type) {
        case DIRECTORY: {
            for (auto itr = FileSet->begin(); itr != FileSet->end();) {
                FilePath = *itr;
                std::cout << "path inside set: " << FilePath << " root File Path: " << rootFile << " " << rootFile.length() << std::endl;
                if (strncmp(FilePath.c_str(), rootFile.c_str(), rootFile.length()) == 0) {
                    std::cout << "filepath exist in root dir" << std::endl;
                    FileSet->erase(FilePath);
                    
                    if (cmd == "store") {
                        if (status == "success") {
                            SuccessReport sr = { FilePath,opr };
                            //process_success.insert(s_id++, sr);
                            process_success[s_id++] = sr;
                            std::cout << "process - content process_success : " << FilePath << std::endl;
                        }
                        else {
                            process_failure[f_id++] = FailureReport{ FilePath,opr,1 };
                            std::cout << "process - content process_failure : " << FilePath << std::endl;
                        }
                    }
                    itr = FileSet->begin();
                }
                else {
                    std::cout << "Doesnt Exist" << std::endl;
                    ++itr;
                }
            }
            break;
        }
        case FILE_TYPE: {
            FileSet->erase(rootFile);
            if (cmd == "store") {
                if (status == "success") {
                    SuccessReport sr = { rootFile,opr };
                    //process_success.insert(s_id++, sr);
                    process_success[s_id++] = sr;
                    std::cout << "process - content process_success : " << rootFile << std::endl;
                }
                else {
                    process_failure[f_id++] = FailureReport{ rootFile,opr,1 };
                    std::cout << "process - content process_failure : " << rootFile << std::endl;
                }
            }
            break;
        }
    }
    return 1;
}

/*CHECKS THE FILE PATH IN CORRESPONDING SET AND BACKUPDIRECTORY DETERMINED BY THE TYPE SPECIFIED*/
int file_check(std::string filepath, std::set<std::string>* fileSet, Type type) {
    int is_in = 0;
    switch (type) {
        case FILE_TYPE: {
            is_in = fileSet->find(filepath) != fileSet->end();
            break;
        }
        case DIRECTORY: {
            std::cout << "Backup dir check" << std::endl;
            std::string temp_fp = g_fp;
            temp_fp.pop_back();
            filepath.erase(1, 1);
            temp_fp.append(filepath);
            std::cout << "Backup Dir Path" << temp_fp << std::endl;
            is_in = dirExists(temp_fp);
            std::cout << "is_in : " << is_in << std::endl;
            break;
        }
        case DIRECTORY_FILE: {
            std::cout << "Backup file check" << std::endl;
            std::string temp_fp = g_fp;
            temp_fp.pop_back();
            filepath.erase(1, 1);
            temp_fp.append(filepath);
            std::cout << "Backup Dir Path" << temp_fp << std::endl;
            std::ifstream ifs(temp_fp);
            is_in = ifs.good();
            std::cout << "is_in : " << is_in << std::endl;
            break;
        }
    }
    return is_in;
}
/*RETURN 0 IF IT IS NOT DIR 1 IF IT IS DIR*/
bool dirExists(const std::string& dirName_in)
{
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}

/*COPIES ENTIRE DIR AND ITS SUB FOLDER*/
bool CopyDirTo(const std::wstring& source_folder, const std::wstring& target_folder)
{
    std::wstring new_sf = source_folder + L"\\*";
    WCHAR sf[MAX_PATH + 1];
    WCHAR tf[MAX_PATH + 1];

    wcscpy_s(sf, MAX_PATH, new_sf.c_str());
    wcscpy_s(tf, MAX_PATH, target_folder.c_str());

    sf[lstrlenW(sf) + 1] = 0;
    tf[lstrlenW(tf) + 1] = 0;

    SHFILEOPSTRUCTW s = { 0 };
    s.wFunc = FO_COPY;
    s.pTo = tf;
    s.pFrom = sf;
    s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NO_UI;
    int res = SHFileOperationW(&s);
    std::cout << "GLE = " << res << std::endl;
    return res == 0;
}

/*DELTES ENTIRE DIR AND ITS SUB FOLDER*/
int DeleteDirectory(const std::string& refcstrRootDirectory, bool bDeleteSubdirectories = true) {
    bool            bSubdirectory = false;       // Flag, indicating whether
                                                 // subdirectories have been found
    HANDLE          hFile;                       // Handle to directory
    std::string     strFilePath;                 // Filepath
    std::string     strPattern;                  // Pattern
    WIN32_FIND_DATA FileInformation;             // File information
    std::ofstream myfile("P:\\restore.txt");
    myfile << "Getting Log" << std::endl;
    strPattern = refcstrRootDirectory + "\\*.*";
    std::wstring wsrefcstrRootDirectory = to_wstring(refcstrRootDirectory);
    std::wstring wsPattern = to_wstring(strPattern);

    hFile = ::FindFirstFile(wsPattern.c_str(), &FileInformation);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FileInformation.cFileName[0] != '.')
            {
                WCHAR* wcfileName = FileInformation.cFileName;
                std::string sFileName = to_string(wcfileName);
                strFilePath.erase();
                strFilePath = refcstrRootDirectory;
                strFilePath.append("\\");
                strFilePath.append(sFileName);
                std::wstring wstFilePath = to_wstring(strFilePath);

                if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (bDeleteSubdirectories)
                    {
                        // Delete subdirectory
                        int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
                        if (iRC)
                            return iRC;
                    }
                    else
                        bSubdirectory = true;
                }
                else
                {
                    // Set file attributes
                    if (::SetFileAttributes(wstFilePath.c_str(),
                        FILE_ATTRIBUTE_NORMAL) == FALSE)
                        return ::GetLastError();

                    // Delete file
                    myfile << "Deletin file " << to_string(wstFilePath.c_str()) << std::endl;
                    if (::DeleteFile(wstFilePath.c_str()) == FALSE)
                        return ::GetLastError();
                }
            }
        } while (::FindNextFile(hFile, &FileInformation) == TRUE);

        // Close handle
        ::FindClose(hFile);

        DWORD dwError = ::GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
            return dwError;
        else
        {
            if (!bSubdirectory)
            {
                // Set directory attributes
                if (::SetFileAttributes(wsrefcstrRootDirectory.c_str(),
                    FILE_ATTRIBUTE_NORMAL) == FALSE)
                    return ::GetLastError();

                // Delete directory
                myfile << "Deleting directory " << to_string(wsrefcstrRootDirectory.c_str()) << std::endl;
                if (::RemoveDirectory(wsrefcstrRootDirectory.c_str()) == FALSE)
                    return ::GetLastError();
            }
        }
    }
    myfile.close();
    return 0;
}

int RestoreContent(std::string src, std::string desc,Type type) {//src->g_fp desc->filePath
    std::wstring wsrc;
    std::wstring wdesc;
    std::string temp_fp;
    std::string temp_desc =desc;
    temp_fp = src;
    temp_fp.pop_back();
    desc.erase(1, 1);
    temp_fp.append(desc);
    std::cout << "Restoring Blocks starts" << std::endl;
    std::cout << "Backup Dir Path" << temp_fp << std::endl;
    std::cout << "src : " << temp_fp << " Desc : " << temp_desc << std::endl;
    wsrc = to_wstring(temp_fp);
    wdesc = to_wstring(temp_desc);
    switch (type)
    {
    case FILE_TYPE:
        CopyFile(wsrc.c_str(), wdesc.c_str(), FALSE);
        break;
    case DIRECTORY:
        CopyDirTo(wsrc, wdesc);
        break;
    default:
        break;
    }

    std::cout << "Restoring Blocks ends" << std::endl;
    return 1;
}

/* PRINT THE GIVEN SET */
void printSet(std::set<std::string> myset)
{
    std::cout << "Set : ";
    for (auto it = myset.begin(); it != myset.end(); ++it)
        std::cout << *it << std::endl;
    std::cout << "\n\n";
}
