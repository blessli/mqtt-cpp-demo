#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/client.h"

using namespace std;
using namespace std::chrono;

const string SERVER_ADDRESS	{ "tcp://120.92.106.222:1883" };
const string CLIENT_ID		{ "111" };

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	mqtt::client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.user_name("emqx2")
		.password("-----------------")
		.keep_alive_interval(seconds(30))
		.automatic_reconnect(seconds(2), seconds(30))
		.clean_session(false)
		.finalize();

	// You can install a callback to change some connection data
	// on auto reconnect attempts. To make a change, update the
	// `connect_data` and return 'true'.
	cli.set_update_connection_handler(
		[](mqtt::connect_data& connData) {
			string newUserName { "emqx2" };
			if (connData.get_user_name() == newUserName)
				return false;

			cout << "Previous user: '" << connData.get_user_name()
				<< "'" << endl;
			connData.set_user_name(newUserName);
			cout << "New user name: '" << connData.get_user_name()
				<< "'" << endl;
			return true;
		}
	);

	const vector<string> TOPICS { "testtopic" };
	const vector<int> QOS { 1 };

	try {
		cout << "Connecting to the MQTT server..." << flush;
		mqtt::connect_response rsp = cli.connect(connOpts);
		cout << "OK\n" << endl;

		if (!rsp.is_session_present()) {
			std::cout << "Subscribing to topics..." << std::flush;
			cli.subscribe(TOPICS, QOS);
			std::cout << "OK" << std::endl;
		}
		else {
			cout << "Session already present. Skipping subscribe." << std::endl;
		}

		// Consume messages

		while (true) {
			auto msg = cli.consume_message();

			if (msg) {
				if (msg->get_topic() == "command" &&
						msg->to_string() == "exit") {
					cout << "Exit command received" << endl;
					break;
				}

				cout << msg->get_topic() << ": " << msg->to_string() << endl;
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

