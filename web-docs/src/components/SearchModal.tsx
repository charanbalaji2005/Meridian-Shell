import React, { useState, useEffect, useRef } from 'react';
import { Search, X, ChevronRight, FileText } from 'lucide-react';
import { DOCS_ARTICLES, DocArticle } from '../data/docsContent';

interface SearchModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSelectArticle: (id: string) => void;
}

export const SearchModal: React.FC<SearchModalProps> = ({
  isOpen,
  onClose,
  onSelectArticle,
}) => {
  const [query, setQuery] = useState('');
  const [selectedIndex, setSelectedIndex] = useState(0);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (isOpen) {
      setTimeout(() => inputRef.current?.focus(), 50);
      setQuery('');
      setSelectedIndex(0);
    }
  }, [isOpen]);

  if (!isOpen) return null;

  const results: { article: DocArticle; matchText: string }[] = Object.values(DOCS_ARTICLES)
    .filter((art) => {
      if (!query.trim()) return true;
      const q = query.toLowerCase();
      return (
        art.title.toLowerCase().includes(q) ||
        art.category.toLowerCase().includes(q) ||
        art.summary.toLowerCase().includes(q) ||
        art.body.toLowerCase().includes(q)
      );
    })
    .map((art) => {
      let matchText = art.summary;
      if (query.trim()) {
        const q = query.toLowerCase();
        const idx = art.body.toLowerCase().indexOf(q);
        if (idx !== -1) {
          const start = Math.max(0, idx - 40);
          const end = Math.min(art.body.length, idx + query.length + 60);
          matchText = '...' + art.body.substring(start, end).replace(/<[^>]*>?/gm, '') + '...';
        }
      }
      return { article: art, matchText };
    })
    .slice(0, 8);

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Escape') {
      onClose();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      setSelectedIndex((prev) => (prev + 1) % Math.max(1, results.length));
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setSelectedIndex((prev) => (prev - 1 + results.length) % Math.max(1, results.length));
    } else if (e.key === 'Enter' && results[selectedIndex]) {
      onSelectArticle(results[selectedIndex].article.id);
      onClose();
    }
  };

  return (
    <div className="search-modal-backdrop" onClick={onClose}>
      <div
        className="search-modal-container"
        onClick={(e) => e.stopPropagation()}
        onKeyDown={handleKeyDown}
      >
        <div className="search-modal-header">
          <Search size={18} className="search-modal-icon" />
          <input
            ref={inputRef}
            type="text"
            className="search-modal-input"
            placeholder="Search documentation, commands, APIs, architecture..."
            value={query}
            onChange={(e) => {
              setQuery(e.target.value);
              setSelectedIndex(0);
            }}
          />
          <button className="search-modal-close" onClick={onClose}>
            <X size={18} />
          </button>
        </div>

        <div className="search-modal-body">
          {results.length === 0 ? (
            <div className="search-empty">
              <p>No documentation found matching &ldquo;{query}&rdquo;</p>
            </div>
          ) : (
            <ul className="search-results-list">
              {results.map(({ article, matchText }, i) => (
                <li
                  key={article.id}
                  className={`search-result-item ${i === selectedIndex ? 'selected' : ''}`}
                  onClick={() => {
                    onSelectArticle(article.id);
                    onClose();
                  }}
                  onMouseEnter={() => setSelectedIndex(i)}
                >
                  <FileText size={16} className="result-icon" />
                  <div className="result-info">
                    <div className="result-title-row">
                      <span className="result-title">{article.title}</span>
                      <span className="result-category">{article.category}</span>
                    </div>
                    <p className="result-snippet">{matchText}</p>
                  </div>
                  <ChevronRight size={14} className="result-arrow" />
                </li>
              ))}
            </ul>
          )}
        </div>

        <div className="search-modal-footer">
          <span className="search-hint"><span>↑</span> <span>↓</span> Navigate</span>
          <span className="search-hint"><span>↵</span> Select</span>
          <span className="search-hint"><span>ESC</span> Close</span>
        </div>
      </div>
    </div>
  );
};
