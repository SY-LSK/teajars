import tkinter as tk
from tkinter import ttk,messagebox
import requests
import json

# 创建主窗口
root = tk.Tk()
root.title("爬虫请求设置")
root.geometry("800x700")

# 应用pywinstyles主题

# URL输入框
url_frame = tk.Frame(root)
url_frame.pack(pady=10, padx=10, fill='x')

url_label = tk.Label(url_frame, text="URL:")
url_label.pack(anchor='w')

url_entry = tk.Entry(url_frame, width=80)
url_entry.pack(pady=5, fill='x')

# 插入默认URL
url_entry.insert(0, "http://127.0.0.1:5000")

# 请求方法选择
method_frame = tk.Frame(root)
method_frame.pack(pady=5, padx=10, fill='x')

method_label = tk.Label(method_frame, text="请求方法:")
method_label.pack(anchor='w')

methods = ["GET", "POST", "PUT", "DELETE", "PATCH"]
method_combo = ttk.Combobox(method_frame, values=methods, width=15, state="readonly")
method_combo.set("GET")
method_combo.pack(pady=5, side='left')

# API选项选择
api_frame = tk.Frame(root)
api_frame.pack(pady=5, padx=10, fill='x')

api_label = tk.Label(api_frame, text="API选项:")
api_label.pack(anchor='w')

options = ["/api/status","/api/set","/api/get","/api/del"]
combo = ttk.Combobox(api_frame, values=options, state="readonly")
combo.set(options[0])
combo.pack(pady=5, side='left')

# 请求体设置
body_frame = tk.LabelFrame(root, text="请求体 (Body)", padx=10, pady=10)
body_frame.pack(pady=10, padx=10, fill='both', expand=True)

body_text = tk.Text(body_frame, height=12, width=70)
body_text.pack(fill='both', expand=True)
body_text.insert(tk.END, '{\n  "key1": "value1",\n  "key2": "value2"\n}')

# 定义发送请求的函数
def send_request():
    try:
        # 获取用户输入
        url = url_entry.get().strip()
        method = method_combo.get()
        api_option = combo.get()
        body_text_content = body_text.get("1.0", tk.END).strip()
        
        # 构造完整URL
        if not url.endswith('/') and not api_option.startswith('/'):
            full_url = url + '/' + api_option
        else:
            full_url = url + api_option
        
        # 解析请求体
        headers = {'Content-Type': 'application/json'}
        data = None
        if body_text_content:
            try:
                data = json.loads(body_text_content)
            except json.JSONDecodeError:
                messagebox.showerror("错误", "请求体JSON格式不正确")
                return
        
        # 发送请求
        if method == "GET":
            response = requests.get(full_url, headers=headers)
        elif method == "POST":
            response = requests.post(full_url, headers=headers, json=data)
        elif method == "PUT":
            response = requests.put(full_url, headers=headers, json=data)
        elif method == "DELETE":
            response = requests.delete(full_url)
        elif method == "PATCH":
            response = requests.patch(full_url, headers=headers, json=data)
        
        # 显示响应
        response_text.delete("1.0", tk.END)
        response_text.insert(tk.END, f"状态码: {response.status_code}\n\n")
        response_text.insert(tk.END, f"响应头:\n{dict(response.headers)}\n\n")
        try:
            response_text.insert(tk.END, f"响应体:\n{json.dumps(response.json(), indent=2, ensure_ascii=False)}")
        except ValueError:
            response_text.insert(tk.END, f"响应体:\n{response.text}")
            
    except requests.exceptions.RequestException as e:
        response_text.delete("1.0", tk.END)
        response_text.insert(tk.END, f"请求错误: {str(e)}")
    except Exception as e:
        response_text.delete("1.0", tk.END)
        response_text.insert(tk.END, f"未知错误: {str(e)}")

# 发送按钮
send_button = tk.Button(root, text="发送请求", bg="#4CAF50", fg="white", font=("Arial", 12), command=send_request)
send_button.pack(pady=10)

# 返回值显示区域
response_frame = tk.LabelFrame(root, text="返回值 (Response)", padx=10, pady=10)
response_frame.pack(pady=10, padx=10, fill='both', expand=True)

response_text = tk.Text(response_frame, height=12, width=70)
response_text.pack(fill='both', expand=True)

root.mainloop()