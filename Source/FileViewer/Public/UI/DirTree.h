// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Widget.h"
#include "DirTree.generated.h"

class SFileView;

/**
 * 
 */
UCLASS()
class FILEVIEWER_API UDirTree : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "FileViewer")
	 	void NavParent();

	UFUNCTION(BlueprintCallable, Category = "FileViewer")
		FText Get_ParentPath() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	UFUNCTION()
	void DirTreeDbClick(const FString& InPath);

protected:
	TSharedPtr<SFileView> MyFileView;
	
};
