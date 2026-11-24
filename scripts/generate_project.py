#!/usr/bin/env python3
"""Generate the dependency-free Xcode project. Run from the repository root."""
from pathlib import Path
import json
objects={}
def add(isa,**kwargs):
    key=f'{len(objects)+1:024X}';objects[key]={'isa':isa,**kwargs};return key
def ref(path,kind,tree='<group>'):
    return add('PBXFileReference',lastKnownFileType=kind,path=path,sourceTree=tree)
def config(name,settings):return add('XCBuildConfiguration',name=name,buildSettings=settings)
def configs(settings):
    return add('XCConfigurationList',buildConfigurations=[config('Debug',{**settings,'GCC_OPTIMIZATION_LEVEL':'0','GCC_PREPROCESSOR_DEFINITIONS':['DEBUG=1','$(inherited)']}),config('Release',{**settings,'GCC_OPTIMIZATION_LEVEL':'3'})],defaultConfigurationIsVisible=0,defaultConfigurationName='Release')
source_paths=['CS80/DSP/Engine.cpp','CS80/AudioUnit/CS80AudioUnit.mm','CS80/UI/CS80ViewController.mm','CS80/App/main.mm','CS80/Tests/main.mm']
headers=['CS80/DSP/PerformancePresets.hpp','CS80/DSP/Engine.hpp','CS80/DSP/Parameters.hpp','CS80/AudioUnit/CS80AudioUnit.h','CS80/UI/CS80ViewController.h','CS80/App/Info.plist','CS80/Extension/Info.plist','CS80/App/CS80.entitlements','CS80/Extension/CS80.entitlements']
files={p:ref(p,'sourcecode.cpp.cpp' if p.endswith('.cpp') else 'sourcecode.cpp.objcpp' if p.endswith('.mm') else 'sourcecode.cpp.h' if p.endswith('.hpp') else 'sourcecode.c.h' if p.endswith('.h') else 'text.plist.xml') for p in source_paths+headers}
frameworks={name:ref(f'System/Library/Frameworks/{name}.framework','wrapper.framework','SDKROOT') for name in ['CoreAudioKit','Cocoa','AudioToolbox','AudioUnit','AVFoundation','CoreMIDI','QuartzCore']}
products={}
for name,kind in [('cs80maths.app','wrapper.application'),('CS80AudioUnit.appex','wrapper.app-extension'),('CS80Tests','compiled.mach-o.executable')]:
    products[name]=add('PBXFileReference',explicitFileType=kind,path=name,sourceTree='BUILT_PRODUCTS_DIR')
productGroup=add('PBXGroup',name='Products',children=list(products.values()),sourceTree='<group>')
frameworkGroup=add('PBXGroup',name='Frameworks',children=list(frameworks.values()),sourceTree='<group>')
mainGroup=add('PBXGroup',children=list(files.values())+[frameworkGroup,productGroup],sourceTree='<group>')
common={'CLANG_ENABLE_MODULES':'YES','CLANG_ENABLE_OBJC_ARC':'YES','CLANG_CXX_LANGUAGE_STANDARD':'c++17','CLANG_CXX_LIBRARY':'libc++','MACOSX_DEPLOYMENT_TARGET':'13.0','SDKROOT':'macosx','CODE_SIGN_STYLE':'Manual','CODE_SIGN_IDENTITY':'-','DEVELOPMENT_TEAM':'','COMBINE_HIDPI_IMAGES':'YES','COPY_PHASE_STRIP':'NO','ENABLE_HARDENED_RUNTIME':'YES','GCC_WARN_64_TO_32_BIT_CONVERSION':'YES','CLANG_WARN_DOCUMENTATION_COMMENTS':'YES','OTHER_CPLUSPLUSFLAGS':['$(inherited)','-Wall','-Wextra','-Wno-unused-parameter'],'HEADER_SEARCH_PATHS':['$(inherited)','$(SRCROOT)/CS80/DSP'],'LD_RUNPATH_SEARCH_PATHS':['$(inherited)','@executable_path/../Frameworks','@executable_path/../../../../Frameworks']}
def target(name,product,productType,sources,settings):
    sourcePhase=add('PBXSourcesBuildPhase',buildActionMask=2147483647,files=[add('PBXBuildFile',fileRef=files[p]) for p in sources],runOnlyForDeploymentPostprocessing=0)
    fwPhase=add('PBXFrameworksBuildPhase',buildActionMask=2147483647,files=[add('PBXBuildFile',fileRef=f) for f in frameworks.values()],runOnlyForDeploymentPostprocessing=0)
    return add('PBXNativeTarget',name=name,productName=name,productReference=products[product],productType=productType,buildConfigurationList=configs({**common,**settings}),buildPhases=[sourcePhase,fwPhase],buildRules=[],dependencies=[])
base=source_paths[:3]
ext=target('CS80AudioUnit','CS80AudioUnit.appex','com.apple.product-type.app-extension',base,{'PRODUCT_BUNDLE_IDENTIFIER':'com.djazz.cs80.AudioUnit','PRODUCT_NAME':'CS80AudioUnit','INFOPLIST_FILE':'CS80/Extension/Info.plist','CODE_SIGN_ENTITLEMENTS':'CS80/Extension/CS80.entitlements','APPLICATION_EXTENSION_API_ONLY':'YES','SKIP_INSTALL':'YES'})
app=target('CS80','cs80maths.app','com.apple.product-type.application',base+['CS80/App/main.mm'],{'PRODUCT_BUNDLE_IDENTIFIER':'com.djazz.cs80','PRODUCT_NAME':'cs80maths','INFOPLIST_FILE':'CS80/App/Info.plist','CODE_SIGN_ENTITLEMENTS':'CS80/App/CS80.entitlements'})
test=target('CS80Tests','CS80Tests','com.apple.product-type.tool',source_paths[:2]+['CS80/Tests/main.mm'],{'PRODUCT_NAME':'CS80Tests','CODE_SIGN_ENTITLEMENTS':'','ENABLE_HARDENED_RUNTIME':'NO'})
embed=add('PBXCopyFilesBuildPhase',buildActionMask=2147483647,dstPath='',dstSubfolderSpec=13,name='Embed App Extensions',files=[add('PBXBuildFile',fileRef=products['CS80AudioUnit.appex'],settings={'ATTRIBUTES':['RemoveHeadersOnCopy']})],runOnlyForDeploymentPostprocessing=0)
objects[app]['buildPhases'].append(embed)
project=add('PBXProject',attributes={'LastUpgradeCheck':'2700','BuildIndependentTargetsInParallel':'YES'},buildConfigurationList=configs(common),compatibilityVersion='Xcode 14.0',developmentRegion='en',knownRegions=['en','Base'],mainGroup=mainGroup,productRefGroup=productGroup,projectDirPath='',projectRoot='',targets=[app,ext,test])
proxy=add('PBXContainerItemProxy',containerPortal=project,proxyType=1,remoteGlobalIDString=ext,remoteInfo='CS80AudioUnit')
dependency=add('PBXTargetDependency',target=ext,targetProxy=proxy);objects[app]['dependencies'].append(dependency)
def emit(x):
    if isinstance(x,dict):return '{ '+''.join(f'{k} = {emit(v)}; ' for k,v in x.items())+'}'
    if isinstance(x,list):return '( '+', '.join(emit(v) for v in x)+' )'
    if isinstance(x,int):return str(x)
    if x in objects:return x
    return json.dumps(x,ensure_ascii=False)
root=Path('CS80.xcodeproj');root.mkdir(exist_ok=True)
(root/'project.pbxproj').write_text('// !$*UTF8*$!\n'+emit({'archiveVersion':1,'classes':{},'objectVersion':56,'objects':objects,'rootObject':project})+'\n')
for scheme,tid,p in [('CS80',app,'cs80maths.app'),('CS80Tests',test,'CS80Tests')]:
    directory=root/'xcshareddata/xcschemes';directory.mkdir(parents=True,exist_ok=True)
    b=f'<BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{tid}" BuildableName="{p}" BlueprintName="{scheme}" ReferencedContainer="container:CS80.xcodeproj"/>'
    (directory/f'{scheme}.xcscheme').write_text(f'''<?xml version="1.0" encoding="UTF-8"?>
<Scheme LastUpgradeVersion="2700" version="1.3"><BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES"><BuildActionEntries><BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES">{b}</BuildActionEntry></BuildActionEntries></BuildAction><LaunchAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" launchStyle="0" useCustomWorkingDirectory="NO" ignoresPersistentStateOnLaunch="NO" debugDocumentVersioning="YES" debugServiceExtension="internal" allowLocationSimulation="YES"><BuildableProductRunnable runnableDebuggingMode="0">{b}</BuildableProductRunnable></LaunchAction><ProfileAction buildConfiguration="Release" shouldUseLaunchSchemeArgsEnv="YES" savedToolIdentifier="" useCustomWorkingDirectory="NO" debugDocumentVersioning="YES"><BuildableProductRunnable runnableDebuggingMode="0">{b}</BuildableProductRunnable></ProfileAction><AnalyzeAction buildConfiguration="Debug"/><ArchiveAction buildConfiguration="Release" revealArchiveInOrganizer="YES"/></Scheme>''')
print('Generated CS80.xcodeproj with app, AUv3 extension, and offline test executable.')
