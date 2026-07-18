// Task 3: Secure File Management System
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct { char name[20], pass[20], group[10]; } User;
typedef struct { char owner[20], group[10]; int perm, enc; } Meta;

// alice owns files; carol shares alice's group (tests the GROUP bits);
// bob is in a different group (tests the OTHER bits)
User users[3] = {{"alice","alice123","admin"}, {"carol","carol123","admin"}, {"bob","bob123","user"}};

void audit(char *u, char *act, char *f, char *res) {
    FILE *fp = fopen("audit.log", "a"); time_t t = time(NULL);
    fprintf(fp, "%.24s user=%s action=%s file=%s result=%s\n", ctime(&t), u, act, f, res); fclose(fp);
}
int login(char *u, char *p, User *out) {
    for (int i = 0; i < 3; i++) if (!strcmp(u, users[i].name) && !strcmp(p, users[i].pass)) { *out = users[i]; return 1; }
    return 0;
}
void xorCipher(char *d, int len, char *key) {  // repeating-key XOR: reversible, encrypts at rest
    for (int i = 0; i < len; i++) d[i] ^= key[i % strlen(key)];
}
// perm is Unix-style octal e.g. 0750: owner=perm/64, group=(perm/8)%8, other=perm%8.
// act is 'r', 'w', or 'x' - covers all three permission classes required by the brief
int checkPerm(Meta *m, User *u, char act) {
    int cls = !strcmp(u->name, m->owner) ? 2 : !strcmp(u->group, m->group) ? 1 : 0;
    int bits = cls == 2 ? m->perm/64 : cls == 1 ? (m->perm/8)%8 : m->perm%8;
    return bits & (act == 'r' ? 4 : act == 'w' ? 2 : 1);
}
int readMeta(char *f, Meta *m) {
    FILE *fp = fopen(f, "r"); if (!fp) return 0;
    fscanf(fp, "%19[^:]:%9[^:]:%d:%d", m->owner, m->group, &m->perm, &m->enc);
    fclose(fp); return 1;
}
int createFile(char *f, User *u, int perm, int enc) {
    FILE *fp = fopen(f, "w"); if (!fp) { audit(u->name, "CREATE", f, "FAILED"); return 0; }
    fprintf(fp, "%s:%s:%d:%d\n", u->name, u->group, perm, enc);
    fclose(fp); audit(u->name, "CREATE", f, "SUCCESS"); return 1;
}
// content is written as one exact-length binary block (not text lines), since
// encrypted bytes can collide with '\n'/'\0' and would corrupt text-mode I/O
int writeFile(char *f, User *u, char *content, char *key) {
    Meta m;
    if (!readMeta(f, &m)) { audit(u->name, "WRITE", f, "DENIED-NOTFOUND"); return 0; }
    if (!checkPerm(&m, u, 'w')) { audit(u->name, "WRITE", f, "DENIED-PERMISSION"); return 0; }
    char buf[512]; strcpy(buf, content); int len = strlen(content);
    if (m.enc) xorCipher(buf, len, key);
    FILE *fp = fopen(f, "a"); fwrite(buf, 1, len, fp); fclose(fp);
    audit(u->name, "WRITE", f, "SUCCESS"); return 1;
}
int readFile(char *f, User *u, char *key) {
    Meta m; char header[64], buf[512];
    if (!readMeta(f, &m)) { audit(u->name, "READ", f, "DENIED-NOTFOUND"); return 0; }
    if (!checkPerm(&m, u, 'r')) { audit(u->name, "READ", f, "DENIED-PERMISSION"); return 0; }
    FILE *fp = fopen(f, "r"); fgets(header, 64, fp);   // skip metadata header line
    long start = ftell(fp); fseek(fp, 0, SEEK_END);
    long len = ftell(fp) - start; fseek(fp, start, SEEK_SET);
    fread(buf, 1, len, fp); buf[len] = '\0';
    if (m.enc) xorCipher(buf, len, key);
    printf("%s\n", buf); fclose(fp);
    audit(u->name, "READ", f, "SUCCESS"); return 1;
}
int deleteFile(char *f, User *u) { Meta m;
    if (!readMeta(f, &m)) { audit(u->name, "DELETE", f, "DENIED-NOTFOUND"); return 0; }
    if (!checkPerm(&m, u, 'w')) { audit(u->name, "DELETE", f, "DENIED-PERMISSION"); return 0; }
    remove(f); audit(u->name, "DELETE", f, "SUCCESS"); return 1;
}
// explicit execute-bit check, reported and logged like any other access attempt
void checkExecute(char *f, User *u) { Meta m; readMeta(f, &m);
    int ok = checkPerm(&m, u, 'x');
    printf("%s execute check on '%s': %s\n", u->name, f, ok ? "ALLOWED" : "DENIED");
    audit(u->name, "EXECUTE", f, ok ? "SUCCESS" : "DENIED-PERMISSION");
}

int main() {
    User alice, carol, bob;
    login("alice", "alice123", &alice);
    login("carol", "carol123", &carol);
    login("bob", "bob123", &bob);

    // owner=rwx, group=r-x, other=--- : lets us test all three classes and bits
    createFile("secret.txt", &alice, 0750, 1);
    writeFile("secret.txt", &alice, "top secret data", "key123");

    printf("-- OWNER (alice) --\n");
    readFile("secret.txt", &alice, "key123");
    checkExecute("secret.txt", &alice);

    printf("-- GROUP (carol, same group as alice) --\n");
    readFile("secret.txt", &carol, "key123");           // group has r-x: read allowed
    if (!writeFile("secret.txt", &carol, "tampered", "key123")) printf("Write denied as expected (group has no w).\n");
    checkExecute("secret.txt", &carol);

    printf("-- OTHER (bob, different group) --\n");
    if (!readFile("secret.txt", &bob, "key123")) printf("Read denied as expected (other has no r).\n");
    checkExecute("secret.txt", &bob);

    deleteFile("secret.txt", &alice);
    return 0;
}