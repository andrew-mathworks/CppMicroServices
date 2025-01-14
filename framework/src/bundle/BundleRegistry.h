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

#ifndef CPPMICROSERVICES_BUNDLEREGISTRY_H
#define CPPMICROSERVICES_BUNDLEREGISTRY_H

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/detail/Threads.h"
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "BundleResourceContainer.h"

namespace cppmicroservices
{

    class CoreBundleContext;
    class Framework;
    class Bundle;
    class BundlePrivate;
    class BundleVersion;
    struct BundleActivator;

    /**
     * Here we handle all the bundles that are known to the framework.
     * @remarks This class is thread-safe.
     */
    class US_ABI_TEST BundleRegistry : private detail::MultiThreaded<>
    {

      public:
        BundleRegistry(CoreBundleContext* coreCtx);
        virtual ~BundleRegistry(void);

        void Init();

        void Clear();

        /**
         * Install a new bundle library.
         *
         * @param location The location to be installed
         * @param caller The bundle performing the install
         * @return A vector of bundles installed
         */
        std::vector<Bundle> Install(std::string const& location,
                                    BundlePrivate* caller,
                                    cppmicroservices::AnyMap const& bundleManifest = cppmicroservices::AnyMap(
                                        cppmicroservices::any_map::UNORDERED_MAP_CASEINSENSITIVE_KEYS));

        /**
         * Remove bundle registration.
         *
         * @param location The location to be removed
         */
        void Remove(std::string const& location, long id);

        /**
         * Get the bundle that has the specified bundle identifier.
         *
         * @param id The identifier of the bundle to get.
         * @return Bundle or null
         *         if the bundle was not found.
         */
        std::shared_ptr<BundlePrivate> GetBundle(long id) const;

        /**
         * Get the bundles that have the specified bundle location.
         *
         * @param location The location of the bundles to get.
         * @return A list of Bundle instances.
         */
        std::vector<std::shared_ptr<BundlePrivate>> GetBundles(std::string const& location) const;

        /**
         * Get all bundles that have the specified bundle symbolic
         * name and version.
         *
         * @param name The symbolic name of bundle to get.
         * @param version The bundle version of bundle to get.
         * @return Collection of BundleImpls.
         */
        std::vector<std::shared_ptr<BundlePrivate>> GetBundles(std::string const& name,
                                                               BundleVersion const& version) const;

        /**
         * Get all known bundles.
         *
         * @return A list which is filled with all known bundles.
         */
        std::vector<std::shared_ptr<BundlePrivate>> GetBundles() const;

        /**
         * Get all bundles currently in bundle state ACTIVE.
         *
         * @return A List of Bundle's.
         */
        std::vector<std::shared_ptr<BundlePrivate>> GetActiveBundles() const;

        /**
         * Try to load any saved framework state.
         * This is done by installing all saved bundles from the local archive
         * copy, and restoring the saved state for each bundle. This is only
         * intended to be executed during the start of the framework.
         *
         */
        void Load();

      private:
        using BundleMap = std::multimap<std::string, std::shared_ptr<BundlePrivate>>;

        // don't allow copying the BundleRegistry.
        BundleRegistry(BundleRegistry const&) = delete;
        BundleRegistry& operator=(BundleRegistry const&) = delete;

        std::vector<Bundle> InstallWithContainer(std::string const& location,
                                     cppmicroservices::AnyMap const& bundleManifest,
                                     std::shared_ptr<BundleResourceContainer> const& resCont);
        std::vector<Bundle> Install0(std::string const& location,
                                     std::shared_ptr<BundleResourceContainer> const& resCont,
                                     std::vector<std::string> const& alreadyInstalled,
                                     cppmicroservices::AnyMap const& bundleManifest);

        void CheckIllegalState() const;

        /** This function populates the res and alreadyInstalled vectors with the appropriate entries so
         * that they can be used by the Install0 call. This was extracted from Install() for convenience.
         *
         * @param range            range containing bundles found at location
         * @param location         the location on disk to look for installed bundles
         * @param bundleManifest   the manifest for the bundle to be installed. Used when creating the
         *                         returned BundleResourceContainer
         * @param res              An output vector of the bundles already installed at location
         * @param alreadyInstalled An output vector of symbolic names of the already installed bundles at
         *                         location.
         */
        std::shared_ptr<BundleResourceContainer> GetAlreadyInstalledBundlesAtLocation(
            std::pair<BundleMap::iterator, BundleMap::iterator> range,
            std::string const& location,
            cppmicroservices::AnyMap const& bundleManifest,
            std::vector<Bundle>& res,
            std::vector<std::string>& alreadyInstalled);

        void DecrementInitialBundleMapRef(cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock& l,
                                          std::string const& location);

        /*
          A struct which contains the necessary objects to utilize condition
          variables. A thread will wait on this WaitCondition if the waitFlag
          is set to true.
        */
        struct WaitCondition
        {
            std::unique_ptr<std::mutex> m;
            std::unique_ptr<std::condition_variable> cv;
            bool waitFlag;

            WaitCondition()
                : m(std::make_unique<std::mutex>())
                , cv(std::make_unique<std::condition_variable>())
                , waitFlag(true)
            {
            }
        };

        std::unordered_map<std::string, std::pair<unsigned int, WaitCondition>> initialBundleInstallMap;

        CoreBundleContext* coreCtx;

        /**
         * Table of all installed bundles in this framework.
         * Key is the bundle location.
         */
        struct : MultiThreaded<>
        {
            BundleMap v;
        } bundles;

        friend class MockedEnvironment;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLEREGISTRY_H
