/*
   Copyright (C) 2019 MIRACL UK Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

     https://www.gnu.org/licenses/agpl-3.0.en.html

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   You can be released from the requirements of the license by purchasing
   a commercial license. Buying such a license is mandatory as soon as you
   develop commercial activities involving the MIRACL Core Crypto SDK
   without disclosing the source code of your own applications, or shipping
   the MIRACL Core Crypto SDK with a closed source product.
*/

/* Boneh-Lynn-Shacham signature 192-bit API */

/* Loosely (for now) following https://datatracker.ietf.org/doc/html/draft-irtf-cfrg-bls-signature-00 */

// Minimal-signature-size variant


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bls192_ZZZ.h"

using namespace XXX;
using namespace YYY;

#ifndef CORE_ARDUINO
static FP8 G2_TAB[G2_TABLE_ZZZ];  // space for precomputation on fixed G2 parameter
#endif

#define CEIL(a,b) (((a)-1)/(b)+1)

/* output u[i] \in F_p */
/* https://datatracker.ietf.org/doc/draft-irtf-cfrg-hash-to-curve/ */
static void hash_to_field(int hash,int hlen,FP *u,octet *DST,octet *M, int ctr)
{
    int i,j,L;
    BIG q,w;
    DBIG dx;
    char okm[256],fd[128];
    octet OKM = {0,sizeof(okm),okm};

    BIG_rcopy(q, Modulus);
    L=CEIL(BIG_nbits(q)+CURVE_SECURITY_ZZZ,8);

    XMD_Expand(hash,hlen,&OKM,L*ctr,DST,M);
    for (i=0;i<ctr;i++)
    {
        for (j=0;j<L;j++)
            fd[j]=OKM.val[i*L+j];
        
        BIG_dfromBytesLen(dx,fd,L);
        BIG_dmod(w,dx,q);
        FP_nres(&u[i],w);
    }
}

/* output u \in F_p 
static void hash_to_base(int hash,int hlen,BIG u,octet *DST,octet *M, int ctr)
{
    int L;
    BIG q;
    DBIG dx;
    char prk[64],okm[128],info[16];
    octet PRK = {0,sizeof(prk),prk};
    octet OKM = {0,sizeof(okm),okm};
    octet INFO = {0,sizeof(info),info};

    BIG_rcopy(q, Modulus);
    L=CEIL(BIG_nbits(q)+CURVE_SECURITY_ZZZ,8);

    OCT_jstring(&INFO,(char *)"H2C");
    OCT_jint(&INFO,ctr,1);
    HKDF_Extract(hash,hlen,&PRK,DST,M);
    HKDF_Expand(hash,hlen,&OKM,L,&PRK,&INFO);

    BIG_dfromBytesLen(dx,OKM.val,L);
    BIG_dmod(u,dx,q);
} */

/* hash a message to an ECP point, using SHA2, random oracle method */
static void BLS_HASH_TO_POINT(ECP *P, octet *M)
{
    FP u[2];
    ECP P1;
    char dst[50];
    octet DST = {0,sizeof(dst),dst};

    OCT_jstring(&DST,(char *)"BLS_SIG_ZZZG1_XMD:SHA384-SSWU-RO-_NUL_");
    hash_to_field(MC_SHA2,HASH_TYPE_ZZZ,u,&DST,M,2);

    ECP_map2point(P,&u[0]);
    ECP_map2point(&P1,&u[1]);

    ECP_add(P,&P1);
    ECP_cfp(P);
    ECP_affine(P);
}

#ifdef CORE_ARDUINO
int ZZZ::BLS_INIT()
{
    return BLS_OK;
}
#else
int ZZZ::BLS_INIT()
{
    ECP4 G;
    if (!ECP4_generator(&G)) return BLS_FAIL;
    PAIR_precomp(G2_TAB, &G);  // should be done just once on start-up
    return BLS_OK;
}
#endif

/* generate key pair, private key S, public key W */

int ZZZ::BLS_KEY_PAIR_GENERATE(octet *IKM, octet* S, octet *W)
{
    int L;
    BIG r,s;
    DBIG dx;
    ECP4 G;
    char salt[20],prk[HASH_TYPE_ZZZ],okm[128];
    octet SALT = {0,sizeof(salt),salt};
    octet PRK = {0,sizeof(prk),prk};
    octet OKM = {0,sizeof(okm),okm};

    BIG_rcopy(r, CURVE_Order);
    L=CEIL(3*CEIL(BIG_nbits(r),8),2);

    if (!ECP4_generator(&G)) return BLS_FAIL;

    OCT_jstring(&SALT,(char *)"BLS-SIG-KEYGEN-SALT-");
    HKDF_Extract(MC_SHA2,HASH_TYPE_ZZZ,&PRK,&SALT,IKM);
    HKDF_Expand(MC_SHA2,HASH_TYPE_ZZZ,&OKM,L,&PRK,NULL);
    BIG_dfromBytesLen(dx,OKM.val,L);
    BIG_dmod(s,dx,r);
    BIG_toBytes(S->val, s);
    S->len = MODBYTES_XXX;
    PAIR_G2mul(&G, s);
    ECP4_toOctet(W, &G, true);
    return BLS_OK;
}


/* Sign message m using private key S to produce signature SIG */

int ZZZ::BLS_CORE_SIGN(octet *SIG, octet *M, octet *S)
{
    BIG s;
    ECP D;
    BLS_HASH_TO_POINT(&D, M);
    BIG_fromBytes(s, S->val);
    PAIR_G1mul(&D, s);
    ECP_toOctet(SIG, &D, true); /* compress output */
    return BLS_OK;
}

/* Verify signature given message M, the signature SIG, and the public key W */
int ZZZ::BLS_CORE_VERIFY(octet *SIG, octet *M, octet *W)
{
    FP24 v;
    ECP4 G, PK;
    ECP D, HM;
    BLS_HASH_TO_POINT(&HM, M);

    ECP_fromOctet(&D, SIG);
	if (!PAIR_G1member(&D)) return BLS_FAIL;
    ECP_neg(&D);

    ECP4_fromOctet(&PK, W);

// Use new multi-pairing mechanism
#ifndef CORE_ARDUINO
    FP24 r[ATE_BITS_ZZZ];

    PAIR_initmp(r);
    PAIR_another_pc(r, G2_TAB, &D);
    PAIR_another(r, &PK, &HM);
    PAIR_miller(&v, r);
#else
//.. or alternatively
    if (!ECP4_generator(&G)) return BLS_FAIL;
    PAIR_double_ate(&v, &G, &D, &PK, &HM);
#endif
    PAIR_fexp(&v);

    if (FP24_isunity(&v)) return BLS_OK;
    return BLS_FAIL;
}

