#ifdef ANDROID
extern "C" int OPENSSL_init_ssl(unsigned long long options, const void *settings);
extern "C" int OPENSSL_init_crypto(unsigned long long options, const void *settings);

__attribute__((constructor)) static void qtOpenSslLinkAnchor()
{
    OPENSSL_init_ssl(0, nullptr);
    OPENSSL_init_crypto(0, nullptr);
}
#endif
