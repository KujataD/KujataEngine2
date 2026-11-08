# 新しいモジュールを作って、ソリューションとAppにつなぐ。
# 使い方はpowershell -ExecutionPolicy Bypass -File Tools\NewModule.ps1 モジュール名。

param(
	[Parameter(Mandatory = $true)]
	[string]$Name
)

$ErrorActionPreference = 'Stop'

# 全構成の名前で、Modules\Module.propsの構成とそろえる。
$configurations = @('Debug', 'Development', 'Release')

# ソリューションに書く、C++のプロジェクトを表す決まった番号。
$cppProjectType = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'

$rootDir = Split-Path -Parent $PSScriptRoot
$moduleDir = Join-Path $rootDir "Modules\$Name"
$solutionPath = Join-Path $rootDir 'KujataEngine2.sln'
$appProjectPath = Join-Path $rootDir 'App\App.vcxproj'

function Stop-WithMessage([string]$message) {
	Write-Host $message -ForegroundColor Red
	exit 1
}

# 改行はLFにそろえて返すので、編集する側はLFだけを考えればよい。
function Read-TextFile([string]$path) {
	return [IO.File]::ReadAllText($path) -replace "`r`n", "`n"
}

# 改行はCRLFで書き、元のファイルにBOMがあれば残す。
function Write-TextFile([string]$path, [string]$text) {
	$hasBom = $false
	if (Test-Path $path) {
		$bytes = [IO.File]::ReadAllBytes($path)
		$hasBom = ($bytes.Length -ge 3) -and ($bytes[0] -eq 0xEF) -and ($bytes[1] -eq 0xBB) -and ($bytes[2] -eq 0xBF)
	}
	$encoding = New-Object Text.UTF8Encoding($hasBom)
	[IO.File]::WriteAllText($path, ($text -replace "`r?`n", "`r`n"), $encoding)
}

# クラス名とファイル名に使うので、C++の名前として書けるものだけを通す。
if ($Name -notmatch '^[A-Za-z][A-Za-z0-9_]*$') {
	Stop-WithMessage "モジュール名は英字で始め、英数字と_だけにしてください: $Name"
}
if (Test-Path $moduleDir) {
	Stop-WithMessage "同じ名前のフォルダがすでにあります: $moduleDir"
}

$solutionText = Read-TextFile $solutionPath
$appProjectText = Read-TextFile $appProjectPath

if ($solutionText.Contains("`"$Name`", ")) {
	Stop-WithMessage "同じ名前のプロジェクトがソリューションにすでにあります: $Name"
}

$guid = [guid]::NewGuid().ToString('B').ToUpper()

# ソリューションの編集は、書き込みの前に全部終わらせて、途中で失敗してもファイルを壊さない。
$projectBlock = "Project(`"$cppProjectType`") = `"$Name`", `"Modules\$Name\$Name.vcxproj`", `"$guid`"`nEndProject`n"
$globalMarker = "`nGlobal`n"
if (-not $solutionText.Contains($globalMarker)) {
	Stop-WithMessage 'ソリューションにGlobalの行が見つかりません。'
}
$solutionText = $solutionText.Replace($globalMarker, "`n$projectBlock" + "Global`n")

$configurationLines = ''
foreach ($configuration in $configurations) {
	$configurationLines += "`t`t$guid.$configuration|x64.ActiveCfg = $configuration|x64`n"
	$configurationLines += "`t`t$guid.$configuration|x64.Build.0 = $configuration|x64`n"
}
$sectionBegin = $solutionText.IndexOf('GlobalSection(ProjectConfigurationPlatforms)')
if ($sectionBegin -lt 0) {
	Stop-WithMessage 'ソリューションに構成の一覧が見つかりません。'
}
$sectionEnd = $solutionText.IndexOf("`tEndGlobalSection", $sectionBegin)
$solutionText = $solutionText.Insert($sectionEnd, $configurationLines)

# Appの参照の一覧の最後に足す。
$lastReferenceEnd = '</ProjectReference>'
$lastReferenceIndex = $appProjectText.LastIndexOf($lastReferenceEnd)
if ($lastReferenceIndex -lt 0) {
	Stop-WithMessage 'App.vcxprojにプロジェクト参照が見つかりません。'
}
$insertIndex = $lastReferenceIndex + $lastReferenceEnd.Length
$newReference = "`n    <ProjectReference Include=`"..\Modules\$Name\$Name.vcxproj`">`n      <Project>$guid</Project>`n    </ProjectReference>"
$appProjectText = $appProjectText.Insert($insertIndex, $newReference)

# ここから下でファイルを書く。
New-Item -ItemType Directory -Path $moduleDir | Out-Null

Write-TextFile (Join-Path $moduleDir "$Name.vcxproj") @"
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Label="Globals">
    <ProjectGuid>$guid</ProjectGuid>
    <RootNamespace>$Name</RootNamespace>
  </PropertyGroup>
  <ItemGroup>
    <ClInclude Include="$Name.h" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="$Name.cpp" />
  </ItemGroup>
  <Import Project="..\Module.props" />
</Project>

"@

# 静的ライブラリはソースが1つもないと作られないので、ひな形のクラスも作る。
Write-TextFile (Join-Path $moduleDir "$Name.h") @"
#pragma once

#include "Core/IModule.h"

namespace KujataEngine {

/// <summary>
/// ${Name}の機能をAppに組み込むモジュール。
/// </summary>
class $Name : public IModule {
public:
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
};

} // namespace KujataEngine

"@

Write-TextFile (Join-Path $moduleDir "$Name.cpp") @"
#include "$Name.h"

namespace KujataEngine {

void ${Name}::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
}

} // namespace KujataEngine

"@

Write-TextFile $solutionPath $solutionText
Write-TextFile $appProjectPath $appProjectText

Write-Host "モジュール $Name を作りました: Modules\$Name" -ForegroundColor Green
Write-Host "使うときはmain.cppで #include `"$Name/$Name.h`" を書いて、app.Add で追加してください。"
Write-Host 'Visual Studioで開いているときは、再読み込みを聞かれたら再読み込みを選んでください。'
