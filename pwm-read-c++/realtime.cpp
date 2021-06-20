
#include <wiringPi.h>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <string>
using namespace std;

// g++ realtime.cpp -lwiringPi -orealtime

/* TODO refactor realtime purposes.
	Future extensions might include:
	 - PWMread
	 - PWMwrite
	 - IR read/write
*/
class PWMComponent {
	private:
		string name;
		int pin;
		chrono::time_point<std::chrono::high_resolution_clock> last_raise;
		bool last_value;
	public:
		PWMComponent(string name, int pin): name(name), pin(pin), last_value(false) {
			pinMode(pin, INPUT);
			pullUpDnControl(pin, PUD_UP);
			cout << "pwm " << name << " initialized with " << pin << endl;
		};
		void tick(chrono::time_point<std::chrono::high_resolution_clock> now) {
			bool value = digitalRead(pin);
			if (value && !last_value) {
				last_raise = now;
				last_value = value;
			} else if (!value && last_value) {
				long diff = chrono::duration_cast<chrono::microseconds>(now - last_raise).count();
				cout << "pwm_" << name << " " << diff << endl;
				last_value = value;
			}
		}
};

int main(int argc, char **argv)
{
	map<string, int> pinmap;
	pinmap["18"] = 1;
	pinmap["23"] = 4;
	pinmap["24"] = 5;
	pinmap["25"] = 6;
	pinmap["8"] = 10;
	pinmap["7"] = 11;
	pinmap["4"] = 7;
	pinmap["17"] = 0;
	pinmap["27"] = 2;
	pinmap["22"] = 3;
	pinmap["10"] = 12;
	pinmap["9"] = 13;
	pinmap["11"] = 14;

   wiringPiSetup();

	vector<PWMComponent> pwm;
	for (int i = 1; i < argc; i++) {
		// TODO: parse realtime components from more complex format
		if (pinmap.find(argv[i]) != pinmap.end()) {
			pwm.push_back(PWMComponent(argv[i], pinmap[argv[i]]));
		}
	}
	auto start = chrono::high_resolution_clock::now();

	for (;;) {
		auto now = chrono::high_resolution_clock::now();
		for (auto &i: pwm) {
			i.tick(now);
		}
	}
	return 0;
}
