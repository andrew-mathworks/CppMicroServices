/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#ifndef CPPMICROSERVICES_BUNDLEMANIFEST_H
#define CPPMICROSERVICES_BUNDLEMANIFEST_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"
#include <mutex>

namespace cppmicroservices
{

    class US_ABI_TEST BundleManifest
    {

      public:
        BundleManifest();
        explicit BundleManifest(AnyMap const& m);

        void Parse(std::istream& is);

        AnyMap const& GetHeaders() const;

        bool Contains(std::string const& key) const;
        Any GetValue(std::string const& key) const;

        Any GetValueDeprecated(std::string const& key) const;
        std::vector<std::string> GetKeysDeprecated() const;

        std::map<std::string, Any> GetPropertiesDeprecated() const;

      private:
        // m_PropertiesDeprecated must be mutable because its content is lazily initialized when
        // one of the const methods GetValueDeprecated(), GetKeysDeprecated(), or
        // GetPropertiesDeprecated() is called.
        mutable std::map<std::string, Any> m_PropertiesDeprecated;
        mutable std::once_flag m_DidCopyDeprecatedProperties;
        AnyMap m_Headers;

        /** copies m_Headers to m_PropertiesDeprecated exactly once per BundleManifest using
         * std::call_once. Needs to be a const method because it's called from other const
         * methods. However, it does modify the values of both mutable fields:
         * m_PropertiesDeprecated and m_DidCopyDeprecatedProperties
         */
        void CopyDeprecatedProperties() const;

        friend class BundleManifestTest_DefaultConstructor_Test;
        friend class BundleManifestTest_ConstructorWithAnyMap_Test;
        friend class BundleManifestTest_ParseValidJson_Test;
        friend class BundleManifestTest_ParseInvalidJsonThrows_Test;
        friend class BundleManifestTest_ParseNonObjectJsonThrows_Test;
        friend class BundleManifestTest_GetValueNonExistentKey_Test;
        friend class BundleManifestTest_CopyDeprecatedProperties_Test;
        friend class BundleManifestTest_GetKeysDeprecated_Test;
        friend class BundleManifestTest_GetPropertiesDeprecated_Test;
        friend class BundleManifestTest_ParseComplexJsonStructure_Test;
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLEMANIFEST_H
