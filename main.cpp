#define WITH_POSIX

#include <coap/coap.h>
#include <stdio.h>
#include <arpa/inet.h> //inet_addr resolve
#include <netinet/in.h>

using namespace std;


static void message_handler(
    struct coap_context_t *ctx, const coap_endpoint_t *local_interface,
    const coap_address_t *remote, coap_pdu_t *sent, coap_pdu_t *received,
    const coap_tid_t id)
{
	unsigned char* data;
	size_t         data_len;

    float code = (float)COAP_RESPONSE_CLASS(received->hdr->code);
    printf("Response Code: %f\n", code);

	//if (COAP_RESPONSE_CLASS(received->hdr->code) == 2)
	//{
		if (coap_get_data(received, &data_len, &data))
		{
			printf("Received: %s\n", data);

		}
	//}
}

#define UBU_IPV6 //loopbak
//#define UBU_IPv4 //loopback
//#define RIVA

int main() //int argc, char* argv[])
{
	coap_context_t*   ctx;
	coap_address_t    dst_addr, src_addr;
	static coap_uri_t uri;
	fd_set            readfds;
	coap_pdu_t*       request;
	unsigned char     get_method = 1;

    printf("Opening socket...\n");
	fflush(stdout);
    coap_address_init(&src_addr);
    coap_address_init(&dst_addr);

#ifdef UBU_IPv4
	/* Prepare coap socket*/
	src_addr.addr.sin.sin_family      = AF_INET;
	src_addr.addr.sin.sin_port        = htons(0);
	src_addr.addr.sin.sin_addr.s_addr = inet_addr("0.0.0.0"); //inet_addr("::1");


	/* The destination endpoint */
    const char* server_uri = "coap://127.0.0.1/Temp";
	dst_addr.addr.sin.sin_family      = AF_INET;
	dst_addr.addr.sin.sin_port        = htons(5683);
	dst_addr.addr.sin.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif // UBU_IPV4

#ifdef UBU_IPV6

	/* Prepare coap socket*/
	sockaddr_in6 sa;
	sa.sin6_family = AF_INET6;
	sa.sin6_port = htons(0);
	inet_pton(AF_INET6, "::", &(sa.sin6_addr));
	src_addr.addr.sin6 = sa;

	/* The destination endpoint */
    const char* server_uri = "coap://[::1]/Temp";
    sockaddr_in6 dsa;
    dsa.sin6_family = AF_INET6;
	dsa.sin6_port = htons(5683);
	inet_pton(AF_INET6, "::1", &(dsa.sin6_addr));
	dst_addr.addr.sin6 = dsa;

#endif // UBU_IPV6

    ctx = coap_new_context(&src_addr);

	/* Prepare the request */
	printf("Preparing request...\n");
	fflush(stdout);
	int r = coap_split_uri((const unsigned char*)server_uri, strlen(server_uri), &uri);
	request            = coap_new_pdu();
	request->hdr->type = COAP_MESSAGE_CON;
	request->hdr->id   = coap_new_message_id(ctx);
	request->hdr->code = get_method;
	coap_add_option(request, COAP_OPTION_URI_PATH, uri.path.length, uri.path.s);

	/* Set the handler and send the request */
	printf("Sending request...\n");
	fflush(stdout);
	coap_register_response_handler(ctx, message_handler);
	coap_send_confirmed(ctx, ctx->endpoint, &dst_addr, request);
	FD_ZERO(&readfds);
	FD_SET( ctx->sockfd, &readfds );
	int result = select( FD_SETSIZE, &readfds, 0, 0, NULL );

	if ( result < 0 ) /* socket error */
	{
        printf("Socket failure!\n");
        fflush(stdout);
		exit(EXIT_FAILURE);

	}

	printf("Reading request...\n");
	fflush(stdout);

	if ( result > 0 && FD_ISSET( ctx->sockfd, &readfds )) /* socket read*/
	{
			coap_read( ctx );
	}

  return 0;
}
