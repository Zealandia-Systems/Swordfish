/*
 * List.h
 *
 * Created: 30/08/2021 7:45:14 pm
 *  Author: smohekey
 */ 

#pragma once

namespace swordfish::utils {
	class Node {
		template<typename T>
		friend class List;
			
	public:
		Node* _prev;
		Node* _next;
		
	protected:
		Node(Node* prev, Node* next) : _prev(prev), _next(next) {
			
		}
		
		void remove() {
			if(_next) {
				_next->_prev = _prev;
			}
			
			if(_prev) {
				_prev->_next = _next;
			}
			
			_next = nullptr;
			_prev = nullptr;
		}
		
		void insertBefore(Node& node) {
			remove();
			
			_prev = node._prev;
			_next = &node;
			
			_prev->_next = this;
			_next->_prev = this;
		}
		
		void insertAfter(Node& node) {
			remove();
			
			_prev = &node;
			_next = node._next;
			
			_prev->_next = this;
			_next->_prev = this;
		}
				
	public:
		Node() : Node(nullptr, nullptr) {
			
		}
	};
	
	template<typename T>
	class ListIterator {
	private:
		Node* _current;
		
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = T;
		using pointer           = value_type*;
		using reference         = value_type&;
		
		ListIterator(Node* current) : _current(current) {
			
		}
		
		reference operator*() {
			Node& current = *_current;
			
			return static_cast<reference>(current);
		}

		pointer operator->() {
			return static_cast<pointer>(_current);
		}
		
		ListIterator<T>& operator++() {
			_current = _current->_next;
			
			return *this;
		}
		
		ListIterator<T> operator++(int) {
			ListIterator<T> tmp = *this;
			
			++(*this);
			
			return tmp;
		}
		
		friend bool operator== (const ListIterator<T>& a, const ListIterator<T>& b) { return a._current == b._current; };
		friend bool operator!= (const ListIterator<T>& a, const ListIterator<T>& b) { return a._current != b._current; };
	};
	
	template<typename T>
	class List : public Node {
	private:
		uint32_t _length;
		
	public:
		using iterator = ListIterator<T>;
		
		List() : Node(this, this), _length(0) {
			
		}
		
		uint32_t length() const {
			return _length;
		}
		
		T* first() const {
			return _next == _prev ? nullptr : static_cast<T*>(_next);
		}
		
		T* last() const {
			return _next == _prev ? nullptr : static_cast<T*>(_prev);
		}
		
		void prepend(T* node) {
			_length++;
			
			static_cast<Node*>(node)->insertBefore(*_next);
		}
		
		void append(T* node) {
			_length++;
			
			static_cast<Node*>(node)->insertAfter(*_prev);
		}
		
		void remove(T* node) {
			_length--;
			
			node->remove();	
		}
		
		T* operator[](uint32_t index) {
			auto node = _next;
			
			while(index-- && node != this) {
				
			}
			
			return static_cast<T*>(node);
		}
		
		ListIterator<T> begin() { return ListIterator<T>(_next); }
		ListIterator<T> end() { return ListIterator<T>(this); }
	};
}