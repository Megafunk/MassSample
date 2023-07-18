// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MassSampleProjectEditorTarget : MassSampleProjectTarget
{
	public MassSampleProjectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
	}
}
