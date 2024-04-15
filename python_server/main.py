import json
import math
from json import JSONEncoder

from flask import Flask, jsonify, request, Request
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
    np_data = get_matrix_from_request(request, "data")
    if np_data is None:
        return "Bad Request: data is None", 400
    dimension = round(math.sqrt(np_data.shape[0]))
    np_data = np_data.reshape([dimension, dimension])
    np_res = np.linalg.eigh(np_data)
    return np_res.eigenvalues.astype(np.double).tobytes()+np_res.eigenvectors.T.astype(np.double).tobytes()

@app.route('/solve', methods=['GET', 'POST'])
def solve():
    np_a = get_matrix_from_request(request, "data_a")
    np_b = get_matrix_from_request(request, "data_b")
    if (np_a is None) or (np_b is None):
        return "Bad Request: data_a or data_b is None", 400
    dimension = round(math.sqrt(np_a.shape[0]))
    np_a = np_a.reshape([dimension, dimension])
    np_b_cols = round(np_b.shape[0]/dimension)
    np_b = np_b.reshape([dimension, np_b_cols])
    np_res = np.linalg.solve(np_a, np_b)
    print(np_res)
    return np_res.astype(np.double).tobytes()


def get_matrix_from_request(request: Request, key: str) -> np.ndarray:
    req_data = request.files[key].stream.read()
    bytes_size = len(req_data)
    data_size = round(bytes_size/8)
    np_data = np.ndarray([data_size], dtype=np.double, buffer=req_data)
    return np_data


if __name__ == '__main__':
    app.run(debug=True)