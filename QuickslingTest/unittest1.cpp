#include "stdafx.h"
#include "CppUnitTest.h"
#include "Conductor.h"
#include "Quicksling.h"
#include "Constants.h"
#include "simple_handler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace QuickslingTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			QBInfo qbi;
			qbi.GetInfoFromQB();
			qbi.LoadConfigYaml();
			qbi.sequence = 15;
			qbi.LoadConfigYaml();
			// TODO: Your test code here
		}

	};
}