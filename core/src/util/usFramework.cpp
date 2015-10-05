/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "usFramework.h"

#include "usCoreModuleContext_p.h"
#include "usFrameworkPrivate.h"
#include "usModuleInfo.h"
#include "usModuleInitialization.h"
#include "usModuleSettings.h"
#include "usModuleUtils_p.h"
#include "usThreads_p.h"

US_BEGIN_NAMESPACE

Framework::Framework(void) : d(new FrameworkPrivate())
{

}

Framework::Framework(std::map<std::string, std::string>& configuration) :
    d(new FrameworkPrivate(configuration))
{
  
}

Framework::~Framework(void)
{

}

void Framework::Initialize(void)
{
  FrameworkPrivate::Lock{d};
  if (d->initialized)
  {
    return;
  }

  ModuleInfo* moduleInfo = new ModuleInfo(US_CORE_FRAMEWORK_NAME);

  void(Framework::*initFncPtr)(void) = &Framework::Initialize;
  void* frameworkInit = NULL;
  std::memcpy(&frameworkInit, &initFncPtr, sizeof(void*));
  moduleInfo->location = ModuleUtils::GetLibraryPath(frameworkInit);

  d->coreModuleContext.bundleRegistry.RegisterSystemBundle(this, moduleInfo);

  d->initialized = true;
}

void Framework::Start() 
{ 
  Initialize();
  Module::Start();
}

void Framework::Stop() 
{
  FrameworkPrivate::Lock lock(d);
  std::vector<Module*> modules(GetModuleContext()->GetModules());
  for (std::vector<Module*>::const_iterator iter = modules.begin();
      iter != modules.end(); 
      ++iter)
  {
    if ((*iter)->GetName() != US_CORE_FRAMEWORK_NAME)
    {
      (*iter)->Stop();
    }
  }

  Module::Stop();
}

void Framework::Uninstall() 
{
  throw std::runtime_error("Cannot uninstall a system bundle."); 
}

std::string Framework::GetLocation() const
{
  // OSGi Core release 6, section 4.6:
  //  The system bundle GetLocation method returns the string: "System Bundle"
  return std::string("System Bundle");
}

void Framework::SetAutoLoadingEnabled(bool enable)
{
  d->coreModuleContext.settings.SetAutoLoadingEnabled(enable);
}

US_END_NAMESPACE
