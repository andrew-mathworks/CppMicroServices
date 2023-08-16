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

#include <chrono>

#include <gtest/gtest.h>

#include <TestInterfaces/Interfaces.hpp>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceTracker.h>
#include <cppmicroservices/servicecomponent/ComponentConstants.hpp>
#include <cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp>

#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/ComponentConfigurationImpl.hpp"
#include "../../src/manager/ReferenceManager.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"
#include "../TestUtils.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

#include "Mocks.hpp"

namespace scr = cppmicroservices::service::component::runtime;
namespace test
{
    // build graph with complex circuluar reference
    /*
    [01, 02]       [03]    [05, 06]  [04] 07
     ||   \\        ||        ||      ||
     04    03       05        01      07
    */
    TEST(TestCircularReference, singleLargeGraph)
    {
        auto framework = cppmicroservices::FrameworkFactory().NewFramework();
        framework.Start();
        auto context = framework.GetBundleContext();
        EXPECT_TRUE(framework);
        auto logger = std::make_shared<cppmicroservices::scrimpl::MockLogger>();
        // The logger should receive 2 Log() calls as a result of the bundle being started.
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, ::testing::_))
            .Times(testing::AtLeast(1));
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, ::testing::_))
            .Times(testing::AtLeast(1));

        // set logging expectations
        auto CircularReference = testing::HasSubstr("Circular Reference: ");
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, CircularReference, testing::_))
            .Times(1);

        auto loggerReg = context.RegisterService<LogService>(logger);

        test::InstallAndStartConfigAdmin(context);
        test::InstallAndStartDS(context);
        // The names of the bundles do matter here. The bundle containing the dependency MUST
        // be stopped after the one providing the dependency. CppMicroServices stores bundles
        // in sorted order by path.

        auto bundleA = test::InstallAndStartBundle(context, "TestBundleCircularComplex");
        ASSERT_TRUE(bundleA);

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference<test::DSGraph02>();
        auto ref3 = context.GetServiceReference<test::DSGraph03>();
        auto ref4 = context.GetServiceReference<test::DSGraph04>();
        auto ref5 = context.GetServiceReference<test::DSGraph05>();
        auto ref6 = context.GetServiceReference<test::DSGraph06>();
        auto ref7 = context.GetServiceReference<test::DSGraph07>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), false);
        ASSERT_EQ(ref2.operator bool(), false);
        ASSERT_EQ(ref3.operator bool(), false);
        ASSERT_EQ(ref5.operator bool(), false);
        ASSERT_EQ(ref6.operator bool(), false);
        ASSERT_EQ(ref7.operator bool(), true);
        ASSERT_EQ(ref4.operator bool(), true);

        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }

    // build graph with complex circuluar reference
    /*
     01--03   04--06--07
     || /     || //
      02       05
    */
    TEST(TestCircularReference, twoSmallGraph)
    {
        auto framework = cppmicroservices::FrameworkFactory().NewFramework();
        framework.Start();
        auto context = framework.GetBundleContext();
        EXPECT_TRUE(framework);
        auto logger = std::make_shared<cppmicroservices::scrimpl::MockLogger>();
        // The logger should receive 2 Log() calls as a result of the bundle being started.
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, ::testing::_))
            .Times(testing::AtLeast(1));
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, ::testing::_))
            .Times(testing::AtLeast(1));

        // set logging expectations
        auto CircularReference = testing::HasSubstr("Circular Reference: ");
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, CircularReference, testing::_))
            .Times(1);

        auto loggerReg = context.RegisterService<LogService>(logger);

        test::InstallAndStartConfigAdmin(context);
        test::InstallAndStartDS(context);
        // The names of the bundles do matter here. The bundle containing the dependency MUST
        // be stopped after the one providing the dependency. CppMicroServices stores bundles
        // in sorted order by path.

        auto bundleA = test::InstallAndStartBundle(context, "TestBundleCircularDouble");
        ASSERT_TRUE(bundleA);

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference<test::DSGraph02>();
        auto ref3 = context.GetServiceReference<test::DSGraph03>();
        auto ref4 = context.GetServiceReference<test::DSGraph04>();
        auto ref5 = context.GetServiceReference<test::DSGraph05>();
        auto ref6 = context.GetServiceReference<test::DSGraph06>();
        auto ref7 = context.GetServiceReference<test::DSGraph07>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), true);
        ASSERT_EQ(ref2.operator bool(), true);
        ASSERT_EQ(ref3.operator bool(), true);
        ASSERT_EQ(ref4.operator bool(), false);
        ASSERT_EQ(ref5.operator bool(), false);
        ASSERT_EQ(ref6.operator bool(), false);
        ASSERT_EQ(ref7.operator bool(), true);

        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }

    // build graph with complex circuluar reference
    /*
      01  - 
     || \\ \
     ||  04 05 
     ||  || //   
     02 --03   
    */
    TEST(TestCircularReference, optionalAndRequiredPath)
    {
        auto framework = cppmicroservices::FrameworkFactory().NewFramework();
        framework.Start();
        auto context = framework.GetBundleContext();
        EXPECT_TRUE(framework);
        auto logger = std::make_shared<cppmicroservices::scrimpl::MockLogger>();
        // The logger should receive 2 Log() calls as a result of the bundle being started.
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, ::testing::_))
            .Times(testing::AtLeast(1));
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, ::testing::_))
            .Times(testing::AtLeast(1));

        // set logging expectations
        auto CircularReference = testing::HasSubstr("Circular Reference: ");
        EXPECT_CALL(*logger, Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, CircularReference, testing::_))
            .Times(1);

        auto loggerReg = context.RegisterService<LogService>(logger);

        test::InstallAndStartConfigAdmin(context);
        test::InstallAndStartDS(context);
        // The names of the bundles do matter here. The bundle containing the dependency MUST
        // be stopped after the one providing the dependency. CppMicroServices stores bundles
        // in sorted order by path.

        auto bundleA = test::InstallAndStartBundle(context, "TestBundleCircularOptReq");
        ASSERT_TRUE(bundleA);

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference<test::DSGraph02>();
        auto ref3 = context.GetServiceReference<test::DSGraph03>();
        auto ref4 = context.GetServiceReference<test::DSGraph04>();
        auto ref5 = context.GetServiceReference<test::DSGraph05>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), false);
        ASSERT_EQ(ref2.operator bool(), false);
        ASSERT_EQ(ref3.operator bool(), false);
        ASSERT_EQ(ref4.operator bool(), false);
        ASSERT_EQ(ref5.operator bool(), true);


        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }

} // namespace test