#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include "mqtt/client.h"

using namespace std;
using namespace std::chrono;

const string SERVER_ADDRESS	{ "tcp://120.92.106.222:1883" };
const string CLIENT_ID		{ "22222" };

constexpr int QOS_0 = 0;
constexpr int QOS_1 = 1;

/////////////////////////////////////////////////////////////////////////////

// Message table function signature
using handler_t = std::function<bool(const mqtt::message&)>;

// Handler for data messages (i.e. topic "data/#")
bool data_handler(const mqtt::message& msg)
{
	cout << msg.get_topic() << ": " << msg.to_string() << endl;
	return true;
}

// Handler for command messages (i.e. topic "command")
// Return false to exit the application
bool command_handler(const mqtt::message& msg)
{
	if (msg.to_string() == "exit") {
		cout << "Exit command received" << endl;
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	mqtt::client cli(SERVER_ADDRESS, CLIENT_ID,
					 mqtt::create_options(MQTTVERSION_5));

	auto connOpts = mqtt::connect_options_builder()
		.user_name("emqx2")
		.password("test_password123")
		.mqtt_version(MQTTVERSION_5)
		.automatic_reconnect(seconds(2), seconds(30))
		.clean_session(false)
		.finalize();

	// Dispatch table to handle incoming messages based on Subscription ID's.
	std::vector<handler_t> handler {
		data_handler,
		command_handler
	};

	try {
		cout << "Connecting to the MQTT server..." << flush;
		// 权限错误：MQTT error [135]: CONNACK return code
		mqtt::connect_response rsp = cli.connect(connOpts);
		cout << "OK\n" << endl;

		if (!rsp.is_session_present()) {
			std::cout << "Subscribing to topics..." << std::flush;

			mqtt::subscribe_options subOpts;
			mqtt::properties props1 {
				{ mqtt::property::SUBSCRIPTION_IDENTIFIER, 1 },
			};
			cli.subscribe("testtopic/#", QOS_1, subOpts, props1);
			std::cout << "OK" << std::endl;
		}
		else {
			cout << "Session already present. Skipping subscribe." << std::endl;
		}

		// Consume messages

		while (true) {
			auto msg = cli.consume_message();

			// Note: In a real app, you'd want to do a lot more error
			// and bounds checking than this.

			if (msg) {
				// Get the subscription ID from the incoming message
				int subId = mqtt::get<int>(msg->get_properties(),
										   mqtt::property::SUBSCRIPTION_IDENTIFIER);

				// Dispatch to a handler function based on the Subscription ID
				if (!(handler[subId-1])(*msg))
					break;
			}
			else if (!cli.is_connected()) {
				cout << "Lost connection" << endl;
				while (!cli.is_connected()) {
					this_thread::sleep_for(milliseconds(250));
				}
				cout << "Re-established connection" << endl;
			}
		}

		// Disconnect

		cout << "\nDisconnecting from the MQTT server..." << flush;
		cli.disconnect();
		cout << "OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

 	return 0;
}

