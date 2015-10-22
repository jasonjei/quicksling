#include "qbXMLRPWrapper.h"
#include "Constants.h"

class ShellUtilities {
	public:
		static int RegisterUICallbacks() {
			qbXMLRPWrapper qb;

			qb.ProcessSubscription(std::wstring(UI_EVENT_SUBSCRIPTION));
			if (qb.GetHasError()) 
				return -2;

			qb.ProcessSubscription(std::wstring(DATA_EVENT_SUBSCRIPTION));
			if (qb.GetHasError())
				return -3;

			return 1;
		}
		static int UnregisterUICallbacks() {
			qbXMLRPWrapper qb;

			qb.ProcessSubscription(std::wstring(UI_EVENT_UNSUBSCRIPTION));
			if (qb.GetHasError())
				return -2;

			qb.ProcessSubscription(std::wstring(DATA_EVENT_UNSUBSCRIPTION));
			if (qb.GetHasError())
				return -3;

			return 1;
		}
};