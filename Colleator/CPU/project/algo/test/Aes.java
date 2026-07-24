import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;

//AES + CBC
public class Aes {
	public static void main(String[] args) throws Exception {
		// 原文:
		String message = "1234567890111";

		// 256位密钥 = 32 bytes Key:
		//byte[] key = "asdfgh012394lj7g89jigdrsv8354kbg".getBytes();
		byte[] key = {-109,97,24,-19,28,49,-4,58,-52, 106,-52,98,-19,84,39,0,97,-40,102,50,-28,32,31, -102,-47,12,-108,57,-15,-66,-59,-120};

		System.out.println("Message: " + message);
		System.out.println("key:"+Arrays.toString(key));

		// 加密:
		byte[] iv={-2, 19, -74, 26, 58, -60, -40, -43, 116, 69, 106, 73, 104, -4, -14, -37};
		byte[] data = message.getBytes();
		byte[] encrypted = encrypt(key, data);
		System.out.println("Encrypted(enc): " + 
				Base64.getEncoder().encodeToString(encrypted));

		String msg = "C95545442162547680F6BE8AD381842F";
		byte[] encrypted2 = hexTobytes(msg);
		System.out.println("encrypted2: " + Arrays.toString(encrypted2));

		// 解密:
		encrypted = join(iv, encrypted2);

		byte[] decrypted = decrypt(key, encrypted);
		System.out.println("Decrypted(dec): " + new String(decrypted));
	}

	public static byte[] hexTobytes(String hex) {
		if (hex.length() < 1) {
			return null;
		} else {
			byte[] result = new byte[hex.length() / 2];
			int j = 0;
			for(int i = 0; i < hex.length(); i+=2) {
				result[j++] = (byte)Integer.parseInt(hex.substring(i,i+2), 16);
			}
			return result;
		}
	}

	// 加密:
	public static byte[] encrypt(byte[] key, byte[] input) throws GeneralSecurityException {
		// 设置算法/工作模式CBC/填充
		Cipher cipher=Cipher.getInstance("AES/CBC/PKCS5Padding");

		// 恢复秘钥对象
		SecretKey keySpec=new SecretKeySpec(key, "AES");

		// CBC模式需要生成一个16 bytes的initialization vector:
		SecureRandom sr=SecureRandom.getInstanceStrong();
		// byte[] iv=sr.generateSeed(16);
		//byte[] iv={-2, 19, -74, 26, 58, -60, -40, -43, 116, 69, 106, 73, 104, -4, -14, -37};
		byte[] iv={-109,123,-112,113,-100,-80,-93,13,-100,-58,-39,112,-109,125,-76,-72};
		System.out.println("iv(content):"+Arrays.toString(iv));
		System.out.println("iv(len):"+iv.length);

		// 初始化秘钥:操作模式、秘钥、IV参数
		IvParameterSpec ivps=new IvParameterSpec(iv);

		cipher.init(Cipher.ENCRYPT_MODE, keySpec,ivps);

		// 加密
		byte[] data=cipher.doFinal(input);
		System.out.println("data(content):"+Arrays.toString(data));

		// IV不需要保密，把IV和密文一起返回:
		return join(iv,data);
	}

	// 解密:
	public static byte[] decrypt(byte[] key, byte[] input) throws GeneralSecurityException {
		// 把input分割成IV和密文:
		byte[] iv=new byte[16];
		byte[] data=new byte[input.length-16];

		System.arraycopy(input, 0, iv, 0, 16);    //IV
		System.arraycopy(input, 16, data, 0, data.length);   //密文
		System.out.println(Arrays.toString(iv));

		try {
		// 解密:
		//Cipher cipher=Cipher.getInstance("AES/CBC/PKCS5Padding"); //密码对象
		Cipher cipher=Cipher.getInstance("AES/CBC/NoPadding"); //密码对象
		SecretKeySpec keySpec=new SecretKeySpec(key, "AES");  //恢复密码
		IvParameterSpec ivps=new IvParameterSpec(iv);    //恢复IV

		byte[] iv1={-2, 19, -74, 26, 58, -60, -40, -43, 116, 69, 106, 73, 104, -4, -14, -37};
		IvParameterSpec ivps1=new IvParameterSpec(iv1);
		String msg = "C95545442162547680F6BE8AD381842F";
		byte[] encrypted2 = hexTobytes(msg);
		System.out.println("encrypted2: " + Arrays.toString(encrypted2));
		
		System.out.println("key:"+Arrays.toString(key));

		// 初始化秘钥:操作模式、秘钥、IV参数
		cipher.init(Cipher.DECRYPT_MODE, keySpec, ivps1);

		// 解密操作
		return cipher.doFinal(encrypted2);
		} catch (Exception e) {
            e.printStackTrace();
            throw new RuntimeException("decrypt fail!", e);
        }
	}

	// 合并数组
	public static byte[] join(byte[] bs1, byte[] bs2) {
		byte[] r=new byte[bs1.length+bs2.length];
		System.arraycopy(bs1, 0, r, 0, bs1.length);
		System.arraycopy(bs2, 0, r, bs1.length, bs2.length);
		return r;
	}
}
