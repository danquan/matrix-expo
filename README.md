# Matrix Exponential with Pthreads

Chương trình song song tích hai ma trận $A, B: A\times B$.

<h1>Yêu cầu:</h1>

- Trình dịch C++ GNU: https://gcc.gnu.org/

<h1>Các ý tưởng nhân ma trận đa luồng</h1>

<h2>pthreads_1.cpp<h2>

- Chỉ có master đọc

Có $p + 1$ threads thì chia ra làm $p$ phần dọc hoặc ngang tùy ý:

```
                    00000000000000
                    00000000000000
                    11111111111111
                    11111111111111
                    ...
                    pppppppppppppp
                    pppppppppppppp
```

Mỗi threads sẽ tính một phần như đã chia; tổng độ phức tạp:

- Thời gian: $O(\frac{n^3}{p})$
- File `pthreads_1.1.cpp` là cách cài đặt khác của phương án này