/*
 * output:

Convert RSA public key into a string an dvice versa
publicKey: Sun RSA public key, 2048 bits
  params: null
  modulus: 18853651626448533042344052742185586831509096183921137436644620443732807152716528158465416708071104899767862289783079092216042499687784322092232163872332358586822678596223733228124113017356896219191227134298362353552882770945818159114272146532048929436504145362418430766823867890113522564795700689158702507402243560009550536419065620409534494384621580364502393563063483223294632627903706549112325066113361455750410642281763368591922729105346933211850575970566025026523917327761707615319008741255611490792106558703015066844972642677443110535667601315009551275389632601989979561472926080344790824117481932026867279062677
  public exponent: 65537
publicKeyString: MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlVmFPS+dIDcJILVZMM9hdQwEiLcHx7SVYF5gOrakPH7ZzilcOjWYcR47qktQAUu97JbLu3h3WPmm1nkgSXU1lVBoqc8pA1BHmzdvMK61A/F77nspDce0QqH5f5kQvYcuQrl+cCUvaTI/3/WBWwRIO2xGKMKRIgiBWDN/HVsqYU2O2pAJnLKQbz9NkkfGNVdzn4H21hi0shCVWCpt80zZkn0gm3oWtCGHOnyszXUOiw7inAdGkNGiZRyiFOUmFNRKLIYM3WiyU1NRGVrjto9NH/E53JdgSyBEu7kkWMLJqNuwj+DNQFu3Qq5VrNxwWggrwhFG+K0y0+Ed+scT003mlQIDAQAB
publicKeyServer: Sun RSA public key, 2048 bits
  params: null
  modulus: 18853651626448533042344052742185586831509096183921137436644620443732807152716528158465416708071104899767862289783079092216042499687784322092232163872332358586822678596223733228124113017356896219191227134298362353552882770945818159114272146532048929436504145362418430766823867890113522564795700689158702507402243560009550536419065620409534494384621580364502393563063483223294632627903706549112325066113361455750410642281763368591922729105346933211850575970566025026523917327761707615319008741255611490792106558703015066844972642677443110535667601315009551275389632601989979561472926080344790824117481932026867279062677
  public exponent: 65537

 */

import android.util.Base64;

import java.security.*;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;

public class Main {
    public static void main(String[] args) throws NoSuchAlgorithmException, InvalidKeySpecException {
        System.out.println("Convert RSA public key into a string an dvice versa");
        // generate a RSA key pair
        KeyPairGenerator keygen = KeyPairGenerator.getInstance("RSA");
        keygen.initialize(2048, new SecureRandom());
        KeyPair keyPair = keygen.generateKeyPair();
        PrivateKey privateKey = keyPair.getPrivate();
        PublicKey publicKey = keyPair.getPublic();
        System.out.println("publicKey: " + publicKey);
        // get encoded form (byte array)
        byte[] publicKeyByte = publicKey.getEncoded();
        // Base64 encoded string
        String publicKeyString = Base64.encodeToString(publicKeyByte, Base64.NO_WRAP);
        System.out.println("publicKeyString: " + publicKeyString);
        // ... transport to server
        // Base64 decoding to byte array
        byte[] publicKeyByteServer = Base64.decode(publicKeyString, Base64.NO_WRAP);
        // generate the publicKey
        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        PublicKey publicKeyServer = (PublicKey) keyFactory.generatePublic(new X509EncodedKeySpec(publicKeyByteServer));
        System.out.println("publicKeyServer: " + publicKeyServer);
    }
}

