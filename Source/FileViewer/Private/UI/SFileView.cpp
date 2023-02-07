// Fill out your copyright notice in the Description page of Project Settings.

#include "SFileView.h"
#include "FileViewer.h"
#include "GenericPlatform/GenericPlatformFile.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

#define _FAKE_ROOT_ "SYSTEM"



class FileListVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory);

	FileListVisitor(TArray<TSharedRef<FFileInfo>>* _InPtr)
		: mp_FileInofs(_InPtr) {};
private:
	FileListVisitor() = delete;

	/** Data */
	TArray<TSharedRef<FFileInfo>>* mp_FileInofs;
};

bool FileListVisitor::Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
{
	if (!mp_FileInofs) return false;

	FString RelativeFilename = FilenameOrDirectory;
	FPaths::MakeStandardFilename(RelativeFilename);
	
	mp_FileInofs->Add(MakeShared<FFileInfo>(FFileInfo(RelativeFilename, bIsDirectory)));
	return true;
}

void SFileView::Construct(const FArguments& InArgs)
{
	mv_FileList =
		SNew(SListView<TSharedRef<FFileInfo>>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&marr_FileInofs)
		.OnGenerateRow(this, &SFileView::On_GenerateRow)
		.OnMouseButtonDoubleClick(this, &SFileView::On_InfoDbClicked);

	ChildSlot[mv_FileList.ToSharedRef()];

	this->OnDoubleClick = InArgs._ListDbClicked;
}

SFileView::~SFileView()
{
}

void SFileView::Refresh_FileList(const FString& InPath)
{
	if (FPaths::IsSamePath(InPath, mstr_CurPath)) return;
	if (!FPaths::IsDrive(InPath) && InPath.EndsWith(TEXT("/")))
	{
		mstr_CurPath = FPaths::GetPath(InPath);
	}
	else
	{
		mstr_CurPath = InPath;
	}

	if (FPaths::IsSamePath(FPaths::GetPath(mstr_CurPath), mstr_CurPath))
	{
		mstr_CurPath = FPaths::GetPath(InPath);
	}

	if (mstr_CurPath.Equals(_FAKE_ROOT_))
	{

        UE_LOG(LogFileViewer, Log, TEXT("[SFileView] Refresh_FileList, fake root %s|%s"), *InPath, *mstr_CurPath);
        marr_FileInofs.Empty();

		/**
		 * @ref: https://stackoverflow.com/questions/286534/enumerating-all-available-drive-letters-in-windows
		 * GetLogicalDrives returns a list of available (read: used) drives as bits in a mask
         * GetLogicalDriveStrings can get you just the list of currently used drive letters.
		 * GetVolumeInformation can be used to get more information about a specific drive.
		 */
        int32 DrivesMask =
#if PLATFORM_WINDOWS
        (int32)GetLogicalDrives()
#else
            0
#endif // PLATFORM_WINDOWS
            ;

        FMenuBuilder MenuBuilder(true, NULL);
        const TCHAR* DriveLetters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        FString Drive = TEXT("A:");

        for (int32 i = 0; i < 26; i++)
        {
            if (DrivesMask & 0x01)
            {
                Drive[0] = DriveLetters[i];
				marr_FileInofs.Add(MakeShared<FFileInfo>(FFileInfo(Drive + TEXT("/"), true)));
			}

            DrivesMask >>= 1;
        }

		mv_FileList->RequestListRefresh();
	}
	else
	{
        FPaths::MakeStandardFilename(mstr_CurPath);
        UE_LOG(LogFileViewer, Log, TEXT("[SFileView] Refresh_FileList %s|%s"), *InPath, *mstr_CurPath);
        marr_FileInofs.Empty();

        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        FileListVisitor tp_FileVisit(&marr_FileInofs);
        PlatformFile.IterateDirectory(*mstr_CurPath, tp_FileVisit);
        mv_FileList->RequestListRefresh();
	}


}

void SFileView::Nav_Parent()
{
	if (mstr_CurPath.Equals(_FAKE_ROOT_))
	{
		return;
	}

	if (FPaths::IsDrive(mstr_CurPath))
	{
		Refresh_FileList(_FAKE_ROOT_);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SFileView] Nav_Parent(): %s"), *mstr_CurPath);
	Refresh_FileList(FPaths::GetPath(mstr_CurPath));
}

void SFileView::On_InfoDbClicked(TSharedRef<FFileInfo> InItem)
{
	if (InItem->IsDirectory)
	{
		Refresh_FileList(InItem->FileName);
	}
	else
	{
		UE_LOG(LogFileViewer, Log, TEXT("[SFileView] On_InfoDbClicked(): %s is not a directory"), *InItem->FileName);
		OnDoubleClick.ExecuteIfBound(InItem->FileName);
	}
}

TSharedRef<ITableRow> SFileView::On_GenerateRow(TSharedRef<FFileInfo> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow< TSharedRef<FFileInfo> >, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->FileName))
			.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
			.ColorAndOpacity(FLinearColor(1, 0, 1, 1))
			.ShadowColorAndOpacity(FLinearColor::Black)
			.ShadowOffset(FIntPoint(-2, 2))
		];
}
