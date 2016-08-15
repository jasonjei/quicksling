#include "qbXMLRPWrapper.h"
#include "Constants.h"
#include "spdlog\spdlog.h"

class ShellUtilities {
	public:
		static int RegisterUICallbacks() {
			qbXMLRPWrapper qb;
			auto l = spdlog::get("quicksling_shell");

			qb.ProcessSubscription(std::wstring(UI_EVENT_SUBSCRIPTION));
			if (qb.GetHasError()) {
				l->error("Register UI Event Subscription error: {}", CW2A(qb.GetErrorMsg().c_str(), CP_UTF8));
				return -2;
			}

			qb.ProcessSubscription(std::wstring(DATA_EVENT_SUBSCRIPTION));
			if (qb.GetHasError()) {
				l->error("Register Data Event Subscription error: {}", CW2A(qb.GetErrorMsg().c_str(), CP_UTF8));
				return -3;
			}

			l->info("Register UI Callbacks successful");
			return 1;
		}
		static int UnregisterUICallbacks() {
			qbXMLRPWrapper qb;
			auto l = spdlog::get("quicksling_shell");

			qb.ProcessSubscription(std::wstring(UI_EVENT_UNSUBSCRIPTION));
			if (qb.GetHasError()) {
				l->error("Unregister UI Event Subscription error: {}", CW2A(qb.GetErrorMsg().c_str(), CP_UTF8));
				return -2;
			}

			qb.ProcessSubscription(std::wstring(DATA_EVENT_UNSUBSCRIPTION));
			if (qb.GetHasError()) {
				l->error("Unregister Data Event Subscription error: {}", CW2A(qb.GetErrorMsg().c_str(), CP_UTF8));
				return -3;
			}

			l->info("Unregister UI Callbacks successful");
			return 1;
		}
};