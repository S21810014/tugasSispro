#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define LISTEN_PORT 1337

int main() {
	//baris-baris berikut merupakan proses standar untuk
    //inisialisasi socket
	int sockListen, sockRecv, i, addrSize;
	struct sockaddr_in myAddr, recvAddr;

	printf("Starting server\n");
	sockListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockListen < 0) {
		printf("socket() failed\n");
		exit(0);
	}

	memset(&myAddr, 0, sizeof(myAddr));

	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons((unsigned short)LISTEN_PORT);

	i = bind(sockListen, (struct sockaddr*)&myAddr, sizeof(myAddr));

	if(i < 0) {
		printf("bind() failed\n");
		exit(0);
	}

	i = listen(sockListen, 5);
	printf("Server listening on port 1337\n");

	if(i < 0) {
		printf("listen() failed\n");
		exit(0);
	}

	addrSize = sizeof(recvAddr);
	sockRecv = accept(sockListen, (struct sockaddr*)&recvAddr, &addrSize);
	printf("Client connected\n");
	
	//kode program utama
	while(!0) {
		//siapkan variabel untuk menerima panjang pesan dari seberang
		int recvLen = 0;
		//terima panjang pesan dari seberang
		recv(sockRecv, &recvLen, sizeof(recvLen), 0);
		//jika panjang pesan adalah 0, maka tidak ada yang dikirim.
		//maka hentikan server
		if(recvLen == 0) 
			break;
		
		//buat array buffer untuk menerima pesan dari seberang, ukuran array
		//berdasarkan panjang pesan yang diterima
		char *buffer = (char*) calloc(sizeof(char), recvLen);
		//variabel untuk mencatat sudah berapa banyak data yang didapatkan
		int recvSeek = 0;
		//selama banyaknya data yang didapatkan tidak setara dengan
		//panjang pesan yang diterima sebelumnya. lakukan penerimaan pesan
		//terus menerus
		while(recvSeek < recvLen)
			recvSeek += recv(sockRecv, buffer+recvSeek, recvLen-recvSeek, 0);
		
		//tampilkan pesan dari seberang
		printf("Client: %s\n", buffer);
		//bersihkan alokasi memori buffer
		free(buffer);

		//tunjukkan prompt untuk server menulis pesan
		printf("Server> ");
		//siapkan buffer sebesar 2048 karakter untuk menampung pesan
		buffer = (char*) calloc(sizeof(char), 2048);
		//mulai menerima input dari user
		fgets(buffer, 2048, stdin);
		//hitung panjang pesan yang ditulis user
		int sendLen = strlen(buffer);
		//kirim panjang pesan ke seberang
		send(sockRecv, &sendLen, sizeof(sendLen), 0);
		//kirim pesan ke seberang
		send(sockRecv, buffer, sendLen, 0);
		//bersihkan kembali alokasi memori buffer
		free(buffer);
	}

	printf("Quitting server\n");

	close(sockRecv);
	close(sockListen);

	return 0;
}
