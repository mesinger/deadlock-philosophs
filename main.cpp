#include <cstdio>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include <string>

/*
 * What are the necessary conditions for deadlocks?
 * mutual exclusion: dh. es gibt mindestens eine ressource die nicht zwischen mehreren threads geshared werden kann
 * hold and wait: ein thread holded gerade eine non-shareable ressource, und benoetigt eine andere non-shareable ressource, die gerade ein anderer thread haelt
 * no preemption: eine geholdede ressource kann nur vom thread selbst released werden
 * circular wait: t1 haltet r1 (ressource 1), t2 haltet r2, t3 haltet r3, t1 braucht r2, t2 braucht r3, t3 braucht r1 -> tn wartet auf tn+1, tn+1 wartet auf tn+2, tn+2 wartet auf t1, jeder thread wartet circular auf einen anderen thread
 *
 * Why does the initial solution lead to a deadlock?
 * mutual exclusion: mutex im Fork
 * hold and wait: Philosopher locked mutex fuer linken fork, und gleich danach den mutex fuer den rechten fork
 * no preemption: Nur der Philosopher unlocked den mutex am fork
 * circular wait: wenn alle Philosopher gleichzeitig ihren linken fork locken, sind wiederum auch alle 'rechten' forks gelocked, dh. jeder Philosoph muss auf seinen rechten nachbar warten, der wiederum auf seinen rechten nachbar, ... -> circular wait
 *
 *
 */

int makeRand(int from, int to) {
	return (from + (rand() % (to - from))) % to;
}

class Fork {
public:
	explicit Fork(int id) : id(id) {
		m = new std::mutex;
	}

	virtual ~Fork() {
		delete m;
	}

	void takeFork() {
		m->lock();
	}

	void releaseFork() {
		m->unlock();
	}

	const int id;

private:
	std::mutex* m = nullptr;
};

class Philosopher {
public:
	Philosopher(int id, int thinkingTime, int eatingTime) : id(id), thinkingTime(thinkingTime), eatingTime(eatingTime) {

	}

	virtual ~Philosopher() = default;

	void thinkAndEat(Fork& leftFork, Fork& rightFork) {

		while (running) {

			auto randomizedThinkingTime = makeRand(0, thinkingTime);
			auto randomizedEatingTime = makeRand(0, eatingTime);

			std::this_thread::sleep_for(std::chrono::milliseconds(randomizedThinkingTime));
			std::printf("philosopher %d finished thinking\n", id);

			leftFork.takeFork();
			std::printf("philosopher %d took left fork %d\n", id, leftFork.id);

			rightFork.takeFork();
			std::printf("philosopher %d took right fork %d\n", id, rightFork.id);

			std::this_thread::sleep_for(std::chrono::milliseconds(randomizedEatingTime));

			leftFork.releaseFork();
			rightFork.releaseFork();
			std::printf("philosopher %d finished eating\n", id);
		}
	}

	void stopThinkingAndEating() {
		running = false;
	}

	const int id;

private:
	const int thinkingTime;
	const int eatingTime;

	bool running = true;
};

int main(int argc, char** argv) {

	srand((unsigned)time(NULL));

	int numberOfPhilosophers = std::stoi(argv[1]);
	int maximalThinkingTime = std::stoi(argv[2]);
	int maximalEatingTime = std::stoi(argv[3]);

	if (numberOfPhilosophers < 2) {
		printf("Number of philosophers has to be >= 2");
		return EXIT_FAILURE;
	}

	std::vector<Philosopher> phils;
	phils.reserve(numberOfPhilosophers);

	for (int i = 0; i < numberOfPhilosophers; i++) {
		phils.emplace_back(i, makeRand(10, maximalThinkingTime), makeRand(10, maximalEatingTime));
	}

	std::vector<Fork> forks;
	forks.reserve(numberOfPhilosophers);

	for (int i = 0; i < numberOfPhilosophers; i++) {
		forks.emplace_back(i);
	}

	std::vector<std::thread> threads;
	threads.reserve(numberOfPhilosophers);

	printf("Enter and key to stop\n");

	for (int i = 0; i < numberOfPhilosophers; i++) {
		threads.emplace_back([&, i] { phils[i].thinkAndEat(forks[i], forks[(i + 1) % numberOfPhilosophers]); });
	}

	std::cin.get();
	printf("Stopping application...");

	for (auto& phil : phils) {
		phil.stopThinkingAndEating();
	}

	for (auto& thread : threads) {
		thread.join();
	}

	return EXIT_SUCCESS;
}
