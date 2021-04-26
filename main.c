#include <gtk/gtk.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define SRVIP "127.0.0.1"
#define SRVPORT 1337

//handle socket yang akan dipakai untuk menerima dan mengirim data
int sockSend;
//deklarasi widget-widget secara global agar dapat diakses semua fungsi
GtkWidget *window, *table, *entryBox, *sendButton, *messageBox;

/*
Fungsi ini akan menghentikan aplikasi
*/
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    gtk_main_quit();s
}

/*
fungsi ini akan mengambil teks yang tertulis dari entryBox dan mengirimkan
nya melalui socket ke seberang, sekaligus juga menampilkan apa yang tertulis
di entryBox ke messageBox
*/
static void sendButtonCallback(GtkWidget *widget, gpointer data) {
    //hitung panjang pesan yang ditulis
    int sendLen = strlen(gtk_entry_get_text(GTK_ENTRY(entryBox)));
    //kirim panjang pesan
    send(sockSend, &sendLen, sizeof(sendLen), 0);
    //kirim pesan
    send(sockSend, gtk_entry_get_text(GTK_ENTRY(entryBox)), sendLen, 0);

    //ambil TextBuffer dari messageBox
    GtkTextBuffer *texBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(messageBox));
    //variabel yang nantinya akan menunjuk ke akhir dari texBuffer
    GtkTextIter endIter;

    //ambil posisi akhir dari texBuffer, lalu simpan ke endIter
    gtk_text_buffer_get_end_iter(texBuffer, &endIter);
    //tambahkan apa yang tertulis di entryBox ke bagian paling terakhir dari teks yang ada
    //di messageBox
    gtk_text_buffer_insert(texBuffer, &endIter, gtk_entry_get_text(GTK_ENTRY(entryBox)), sendLen);
    //ambil kembali posisi terakhir terbaru dari texBuffer
    gtk_text_buffer_get_end_iter(texBuffer, &endIter);
    //tambahkan karakter 'baris baru' agar nantinya pesan selanjutnya akan
    //tertulis di baris yang baru
    gtk_text_buffer_insert(texBuffer, &endIter, "\n", 1);
}

/*
fungsi ini akan berjalan berulang ulang selama program belum dihentikan.

fungsi ini bertugas untuk mendapatkan pesan yang dikirimkan dari seberang
dan menampilkannya ke messageBox
*/
gboolean networkLoop(GtkWidget *widget, GdkFrameClock *clock, gpointer data) {
    //variabel untuk menerima panjang pesan dari seberang
    int recvLen = 0;
    //terima panjang pesan dari seberang
    recv(sockSend, &recvLen, sizeof(recvLen), 0);
    //jika panjang pesan yang diterima ialah 0
    //berarti tidak ada pesan yang dikirimkan.
    //maka langsung return 1
    if(recvLen == 0)
        return 1;

    //persiapkan array buffer dengan ukuran berdasarkan panjang pesan dari
    //seberang untuk menyimpan pesan tersebut.
    char *buffer = (char*) calloc(sizeof(char), recvLen);
    //recvSeek digunakan untuk menghitung sudah berapa banyak data
    //yang didapakan dari seberang
    int recvSeek = 0;
    //selama data yang didapatkan belum sebanyak atau sepanjang jumlah
    //data yang dikirimkan dari seberang, terus lakukan penerimaan
    while(recvSeek < recvLen)
        recvSeek += recv(sockSend, buffer+recvSeek, recvLen-recvSeek, 0);

    //jika panjang data buffer sudah sama dengan panjang pesan yang dikirimkan
    //dari seberang, berarti semua data sudah didapatkan.
    //
    //maka tampilkan data pesan tersebut ke messageBox
    if(strlen(buffer) == recvLen) {
        g_print("Server: %s\n", buffer);

        //ini hal yang sama seperti pada fungsi sebelumnya
        GtkTextBuffer *texBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(messageBox));
        GtkTextIter endIter;
        gtk_text_buffer_get_end_iter(texBuffer, &endIter);
        gtk_text_buffer_insert(texBuffer, &endIter, buffer, recvLen);
    }

    //bersihkan alokasi memori buffer
    free(buffer);
    return 1;
}

int main(int argc, char **argv) {
    //baris-baris berikut merupakan proses standar untuk
    //inisialisasi socket
    int i;
	struct sockaddr_in addrSend;

	sockSend = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockSend < 0) {
		printf("socket() failed\n");
		exit(0);
	}

	memset(&addrSend, 0, sizeof(addrSend));

	addrSend.sin_family = AF_INET;
	addrSend.sin_addr.s_addr = inet_addr(SRVIP);
	addrSend.sin_port = htons((unsigned short)SRVPORT);

	i = connect(sockSend, (struct sockaddr*)&addrSend, sizeof(addrSend));
    
	if(i < 0) {
		printf("connect() failed\n");
		exit(0);
	}

    //khusus pada baris ini, kita mengatur agar socket kita tidak
    //menghambat jalannya program karena menunggu data dari seberang
    //dengan cara mengganti socket tersebut menjadi socket Non-Blocking
    fcntl(sockSend, F_SETFL, O_NONBLOCK);

    //inisialiasi library GTK
    gtk_init(&argc, &argv);

    //bangun window GTK baru
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //ketika user menekan tombol X di window GTK, program akan memanggil
    //fungsi delete_event
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
    //atur batas tepian window menjadi 5
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);

    //buat sebuah tabel yang akan digunakan untuk meletakkan User Interface kita
    table = gtk_table_new(4, 2, TRUE);
    gtk_container_add(GTK_CONTAINER(window), table);

    //buat tombol untuk mengirim pesan
    sendButton = gtk_button_new_with_label("Send");
    gtk_table_attach_defaults(GTK_TABLE(table), sendButton, 1, 2, 3, 4);
    //buat agar ketika tombol ditekan, fungsi sendButtonCallback akan dijalankan
    g_signal_connect(sendButton, "clicked", G_CALLBACK(sendButtonCallback), NULL);

    //buat sebuah textbox untuk mengisi pesan yang akan dikirim
    entryBox = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), entryBox, 0, 1, 3, 4);

    //buat sebuah messageBox untuk menampilkan pesan yang terkirim dan yang
    //diterima
    messageBox = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(messageBox), FALSE);
    gtk_table_attach_defaults(GTK_TABLE(table), messageBox, 0, 2, 0, 3);

    //tampilkan semua elemen-elemen User Interface tersebut
    gtk_widget_show(table);
    gtk_widget_show(sendButton);
    gtk_widget_show(entryBox);
    gtk_widget_show(messageBox);
    gtk_widget_show(window);

    //buat agar fungsi networkLoop dipanggil secara berulang-ulang
    //selama program berjalan
    gtk_widget_add_tick_callback(window, networkLoop, NULL, NULL);
    
    //jalankan looping utama GTK
    gtk_main();
    return 0;
}
