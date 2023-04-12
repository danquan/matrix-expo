# Matrix Exponential using MultiProcessing

<h1>Các ý tưởng nhân ma trận đa luồng</h1>

<h2>1. Chia nhỏ bảng ma trận kết quả</h2>
- Có p + 1 processes thì chia ra làm p phần dọc hoặc ngang tùy ý:

                    00000000000000
                    00000000000000
                    11111111111111
                    11111111111111
                    ...
                    pppppppppppppp
                    pppppppppppppp

Mỗi process sẽ tính một phần như đã chia; tổng độ phức tạp:
    - Thời gian: n^3 / p
    - Vận chuyển: (mn / p + mn) [Master -> Worker] + (mn / p) [Worker -> Master]

<h2>2. Giữ nguyên bảng ma trận kết quả, chia nhỏ ma trận A, B:</h2>

                        0000000000000
                        0000000000000
                        1111111111111
                        1111111111111
                        ...
                        ppppppppppppp
                        ppppppppppppp

0011...pp               
0011...pp
0011...pp
0011...pp
0011...pp
0011...pp
0011...pp

Mỗi process sẽ tính (ma trận con của A) * (ma trận con của B) tương ứng mà nó quản lí
Sau đó tính tổng chập của các ma trận mà các process trả về, thu được ma trận đáp án.

Độ phức tạp:
    - Thời gian: n^3 / p
    - Vận chuyển: (mn / p + mn / p) [Master -> Worker] + (mn) [Worker -> Master]

Dự đoán cách chia 2 tốt hơn cách chia 1 (Do master phải gửi đi ít thông tin hơn, nên thời gian gửi đi của master ít hơn, còn các worker gửi về song song)

<h2>3. Chia nhỏ ma trận đáp án theo cả dọc và ngang</h2>
- q = sqrt(p)
- Ma trận sẽ trông kiểu:

    001122...qq
    001122...qq
    llrr.....hh
    llrr.....hh

- Tức là chia nó thành q * q ma trận nhỏ hơn, sau đó mỗi process quản lí 1 phần và nhân lại
- Ưu điểm: 
    - Tổng độ phức tạp bộ nhớ gửi đi là 2mn/q + mn/p
- Nhược điểm:
    - Không tận dụng được hết các process (Do số process đôi khi không phải số chính phương)
    - Cài đặt async rất khó
-----------------------------------------------------
<h1>Analysis Matrix_Expo</h1>

Bộ test với m, n = 1000 và a[i][j] \in [-1, 1]
Async: 2899ms
Sync: 2881ms

Bộ test với m,n = 1000 và a[i][j] \in [-1000, 1000]
Async: 3246ms
Sync: 3183ms

Bộ test với m,n = 1000 và a[i][j] \in [-1e9, 1e9] (testMatrix_test_1000_1000_1e9.INP)
    Divide idea 1:
        Sync: (2 processes): 5722ms, (3 processes): 4650ms, (4 processes): 5600ms, (5 processes): 10752ms
        Async: (2 processes): 5451ms, (3 processes): 5110ms, (4 processes): 5651ms, (5 processes): 7347ms
    Divide idea 2:
        Async: (2 processes): 4389ms, (3 processes): 3741ms, (4 processes): 4274ms, (5 processes) : 4128ms
        Sync: (2 processes): 4508ms, (3 processes): 3997ms, (4 processes): 4052ms, (5 processes) : 4112ms 

----------------------------------------------------
