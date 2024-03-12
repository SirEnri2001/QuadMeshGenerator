import json
import math
from json import JSONEncoder

from flask import Flask, jsonify, request
import numpy as np

app = Flask(__name__)

class NumpyArrayEncoder(JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return JSONEncoder.default(self, obj)
@app.route('/returnjson', methods=['GET'])
def ReturnJSON():
    if (request.method == 'GET'):
        data = {
            "Modules": 15,
            "Subject": "Data Structures and Algorithms",
        }

        return jsonify(data)
@app.route('/add_message', methods=['GET', 'POST'])
def add_message():
    data = np.array(request.json["matrix"])
    print(data)
    return json.dumps(data, cls=NumpyArrayEncoder)

@app.route('/eigh', methods=['GET', 'POST'])
def svd():
    req_data = request.get_data()
    bytes_target = bytes("Content-Type: text/plain", 'utf-8')
    ind = req_data.find(bytes_target)
    data_size = 0
    if not ind == -1:
        parsed_string = (req_data[ind + len(bytes_target)+4:ind + len(bytes_target)+40]).decode("utf-8")
        data_size = int(parsed_string.split("\r\n")[0])
    bytes_data_target = bytes("Content-Type: application/octet-stream", 'utf-8')
    ind = req_data.find(bytes_data_target)+4+len(bytes_data_target)
    data = req_data[ind:ind+data_size]
    dimension = round(math.sqrt(data_size/8))
    np_data = np.ndarray([dimension, dimension], dtype=np.double, buffer=data)
    np_res = np.linalg.eigh(np_data)
    return np_res.eigenvalues.astype(np.double).tobytes()+np_res.eigenvectors.T.astype(np.double).tobytes()
    # json_data = request.json
    # resp = {}
    # for key, value in json_data.items():

    # print(resp)
    # return resp


if __name__ == '__main__':
    app.run(debug=True)