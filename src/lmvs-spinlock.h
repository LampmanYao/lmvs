#pragma once

typedef int lmvs_spinlock_t;

static inline void
lmvs_spinlock_init(lmvs_spinlock_t* lock) {
	*lock = 0;
}

static inline void
lmvs_spinlock_lock(lmvs_spinlock_t* lock) {
	while (__sync_lock_test_and_set(lock, 1));
}

static inline int
lmvs_spinlock_trylock(lmvs_spinlock_t* lock) {
	return __sync_lock_test_and_set(lock, 1) == 0;
}

static inline void
lmvs_spinlock_unlock(lmvs_spinlock_t* lock) {
	__sync_lock_release(lock);
}

