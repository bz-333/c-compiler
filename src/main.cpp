#include <iostream>

#include "../external/BowenFu/matchit.h"

constexpr int32_t factorial(int32_t n)
{
    using namespace matchit;
    assert(n >= 0);
    return match(n)(
        pattern | 0 = 1,
        pattern | _ = [n] { return n * factorial(n - 1); }
    );
}

int main() {
	std::cout << factorial(5) << '\n';
	return 0;
}
