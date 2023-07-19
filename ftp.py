import base64
from flask import Flask, request

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload_image():
    try:
        # Get the Base64-encoded image data from the JSON payload
        json_data = request.get_json()
        base64_data = json_data.get('image', '')

        # Decode the Base64 data back to binary
        image_data = base64.b64decode(base64_data)

        # Save the image to a file (you can customize the file path as needed)
        with open('uploads/image.jpg', 'wb') as f:
            f.write(image_data)

        return 'Image uploaded successfully.', 200

    except Exception as e:
        return str(e), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
