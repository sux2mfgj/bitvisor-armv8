#ifndef _TEST_H_
#define _TEST_H_

#ifdef CONFIG_CGREEN_TEST
#define test_static
#else
#define test_static static
#endif

#endif // _TEST_H_
