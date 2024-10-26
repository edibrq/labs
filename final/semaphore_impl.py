import threading
import time
import random

class PhoneNetwork:
    def __init__(self, participants):
        self.participants = participants
        self.confirmation = False
        self.confirmation_semaphore = threading.Semaphore(1)
        self.phone_semaphores = {p: threading.Semaphore(1) for p in participants}
        self.confirmation_source = {}

    """
    Semaphore is like a phone

    phone_semaphores:
    {
        'Poluekt' : Semaphore1,
        'Grandma1': Semaphore2, 
        'Grandma2': Semaphore3, 
        'Mother': Semaphore4
    }

    phone_semaphores["Poluekt"] -> Semaphore1
    """

    def make_call(self, caller, callee):
        caller_phone = self.phone_semaphores[caller] # caller "Grandma1" -> semaphore
        calle_phone = self.phone_semaphores[callee]

        acquired_caller = caller_phone.acquire(blocking=False) # занят ты нет?
        acquired_callee = calle_phone.acquire(blocking=False)

        if (acquired_caller and acquired_callee) == True:
            try:
                print(f"{caller} звонит {callee}.")
                time.sleep(random.uniform(0.1, 0.3))
                return True
            finally:
                self.phone_semaphores[callee].release()
                self.phone_semaphores[caller].release()
        else:
            if acquired_caller:
                self.phone_semaphores[caller].release()
            if acquired_callee:
                self.phone_semaphores[callee].release()
            return False

    def poluekt_activity(self):
        time.sleep(random.uniform(0.1, 0.5))
        print("Полуэкт пытается позвонить...")

        while True:
            for person in self.participants[1:]:
                success = self.make_call('Poluekt', person)
                if success:
                    self.confirmation_semaphore.acquire()
                    try:
                        self.confirmation = True
                        self.confirmation_source[person] = 'Poluekt'
                    finally:
                        self.confirmation_semaphore.release()
                    print(f"Полуэкт сообщил {person}, что он в порядке.")
                    return
            time.sleep(random.uniform(0.1, 0.5))
            print("Полуэкт пытается снова позвонить...")

    def member_activity(self, name):
        others = [p for p in self.participants if p != name and p != 'Poluekt']

        while True:
            self.confirmation_semaphore.acquire()
            try:
                if self.confirmation:
                    if name in self.confirmation_source:
                        print(f"{name} получил(а) подтверждение от {self.confirmation_source[name]} и прекращает звонки.")
                        break
            finally:
                self.confirmation_semaphore.release()

            random.shuffle(others)
            for other in others:
                success = self.make_call(name, other)
                if success:
                    print(f"{name} разговаривает с {other}.")
                    self.confirmation_semaphore.acquire()
                    try:
                        if self.confirmation:
                            if other in self.confirmation_source:
                                self.confirmation_source[name] = other
                                print(f"{name} узнал(а), что Полуэкт в порядке от {other}.")
                                return
                            else:
                                self.confirmation_source[name] = other
                    finally:
                        self.confirmation_semaphore.release()
                    print(f"{name} закончила разговор с {other}.")
            time.sleep(random.uniform(0.1, 0.2))

# Main part of the program
num_of_girlfriends = int(input("Введите количество подруг: "))
participants = ['Poluekt', 'Grandma1', 'Grandma2', 'Mother'] + [f"Girlfriend{i}" for i in range(1, num_of_girlfriends + 1)]

network = PhoneNetwork(participants)
threads = []

t_poluekt = threading.Thread(target=network.poluekt_activity)
threads.append(t_poluekt)

for person in participants[1:]:
    t = threading.Thread(target=network.member_activity, args=(person,))
    threads.append(t)

for t in threads:
    t.start()

for t in threads:
    t.join()

print("Все участники получили подтверждение о Полуэкте.")