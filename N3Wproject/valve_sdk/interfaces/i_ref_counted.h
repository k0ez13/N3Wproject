#pragma once
class i_ref_counted {
private:
	volatile long refCount;

public:
	virtual void destructor(char bDelete) = 0;
	virtual bool on_final_release() = 0;

	void unreference() {
		if (InterlockedDecrement(&refCount) == 0 && on_final_release()) {
			destructor(1);
		}
	}
};