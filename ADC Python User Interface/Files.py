""" File module to work with files """
import io

class File:

    @classmethod
    def read_module(self, bytes_from_module:bytes, base64=False):
        if base64 is True:
            import base64
            try:
                return io.BytesIO(base64.b64decode(bytes_from_module)).getvalue()
            except Exception:
                raise BaseException("Failed to return Base64 decoded file-like object")
        else:
            try:
                value = io.BytesIO(bytes_from_module).getvalue()
                return value 
            except Exception as e:
                raise BaseException("Failed to return file-like object", e)

if __name__ == "__main__":
    pass