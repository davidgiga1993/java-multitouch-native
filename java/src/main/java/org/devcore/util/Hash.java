package org.devcore.util;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Hash
{
	public static byte[] md5(InputStream input)
	{
		MessageDigest md;
		try
		{
			md = MessageDigest.getInstance("MD5");
			DataInputStream dis = new DataInputStream(input);
			byte[] data = new byte[input.available()];
			dis.readFully(data);
			md.update(data);
			return md.digest();
		}
		catch (NoSuchAlgorithmException | IOException e)
		{
			return null;
		}
	}

	public static String bytesToHex(byte[] hashInBytes, int maxLength)
	{
		StringBuilder sb = new StringBuilder();
		int pos = 0;
		for (byte hashInByte : hashInBytes)
		{
			sb.append(Integer.toString((hashInByte & 0xff) + 0x100, 16).substring(1));
			pos++;
			if (pos > maxLength)
			{
				break;
			}
		}
		return sb.toString();

	}
}
